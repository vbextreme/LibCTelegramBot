#include <tgb/api.h>
#include <tgb/terminale.h>
#include <curl/curl.h> 
#include "jsmn.h"


#define HTTP_BUFFER 512
#define HTTP_MODE_DINAMIC 0x01

#define TGB_PARSE_USER_NTOK   (26)
#define TGB_PARSE_UPDATE_NTOK (4096)
#define TGB_PARSE_FILE_NTOK   (16)
#define TGB_PARSE_CHAT_NTOK   (30)

/*TODO
 * 0. tgbUpdate CONST!
 * 1. null update optimization
 * 2. keep alive curl buffer on thread 
 * 3. query photo is array
 * 4. entity
 * 5. game
 * 6. inline/result
 * 7. callback
*/

/******************************************************************************************/
/******************************************************************************************/
/******************************************************************************************/
/************************************ GENERIC *********************************************/
/******************************************************************************************/
/******************************************************************************************/
/******************************************************************************************/

err_t file_exist(char_t* name)
{
	file_h* f = fopen(name, "r");
	if( f ) fclose(f);
	return (err_t)f;
}

char_t* i64toa(char_t* d, int64_t s)
{
	sprintf(d,"%lld",s);
	return d;
}


/******************************************************************************************/
/******************************************************************************************/
/******************************************************************************************/
/********************************* CALLBACK CURL ******************************************/
/******************************************************************************************/
/******************************************************************************************/
/******************************************************************************************/

__private size_t http_recv(void* ptr, size_t size, size_t nmemb, void* userp) 
{
	dbg("");
	
	httpBuffer_s* buf = (httpBuffer_s*)userp;
	uint_t maxs = size * nmemb;
	
	if ( buf->size <= buf->rem + maxs )
	{
		dbg("warning: incoming(%u) > buffer(%u)", buf->rem + maxs, buf->size);
		dbg("info: allocate new buffer");
		uint_t nwSize = buf->size + maxs + buf->block;
		char_t* nwBuff = NEWS(char_t, nwSize);
		iassert( nwBuff != NULL );
		memcpy(nwBuff, buf->data, buf->rem);
		free(buf->data);
		buf->data = nwBuff;
		buf->pdata = nwBuff + buf->rem;
		buf->size = nwSize;
	}
	
	dbg("info: copy new data dest[%u][%u] src[%u]", buf->size, buf->rem, maxs);
	memcpy(buf->pdata, ptr, maxs);
	buf->rem += maxs;
	buf->pdata += maxs;
	*buf->pdata = 0;
	
	return maxs;
}

__private size_t http_recv_file(void* ptr, size_t size, size_t nmemb, void* userp) 
{
	dbg("");
	file_h* f = (file_h*)userp;
	size_t ret = fwrite(ptr, size, nmemb, f);
	dbg("write into file %d", ret);
	return ret;
}

/******************************************************************************************/
/******************************************************************************************/
/******************************************************************************************/
/*********************************** HTTP OBJECT ******************************************/
/******************************************************************************************/
/******************************************************************************************/
/******************************************************************************************/

#define http_init() curl_global_init(CURL_GLOBAL_DEFAULT)
#define http_destroy() curl_global_cleanup()
#define http_err_str() curl_easy_strerror(errno)
__always_inline __private int_t http_errno(void) { return errno; }

#define http_form_init() curlHttpPost_s* __post = NULL; curlHttpPost_s* __last = NULL;
#define http_form_add(C, V) curl_formadd( &__post, &__last, CURLFORM_COPYNAME, C, CURLFORM_COPYCONTENTS, V, CURLFORM_END)
#define http_form_add_file(C, V) curl_formadd( &__post, &__last, CURLFORM_COPYNAME, C, CURLFORM_FILE, V, CURLFORM_END)
#define http_form_add_int64(C, V) do{ char_t v[32]; sprintf(v, "%lld", V); http_form_add(C, v);}while(0)
#define http_form_add_float(C, V) do{ char_t v[128]; sprintf(v, "%f", V); http_form_add(C, v);}while(0)
#define http_form_add_bool(C, V) do{ http_form_add(C, (V==0) ? "false" : "true");}while(0)
#define http_form_add_member(S, M) http_form_add(#M, S->M)
#define http_form_add_file_member(S, M) http_form_add_file(#M, S->M)
#define http_form_add_int64_member(S, M) http_form_add_int64(#M, S->M)
#define http_form_add_float_member(S, M) http_form_add_float(#M, S->M)
#define http_form_add_bool_member(S, M) http_form_add_bool(#M, S->M)
#define HTTP_POST __post

__private err_t http_post(httpBuffer_s* out, uint_t bufferBlock, char_t* url, curlHttpPost_s* post, bool_t ssl)
{
	dbg("");
	
	if ( 0 == out->data )
	{
		dbg("create new buffer");
		out->block = bufferBlock;
		out->rem = 0;
		out->size = bufferBlock;
		out->pdata = out->data = NEWS(char_t, bufferBlock);
		iassert( out->data != NULL );
	}
	else
	{
		dbg("reuse buffer");
		out->rem = 0;
		out->pdata = out->data;
	}
	
	CURL* h;
	if ( !(h = curl_easy_init()) )
	{
		dbg("error: curl init :: %s", http_err_str());
		return -1;
	}
	
	curl_easy_setopt(h, CURLOPT_URL, url);
	
	if ( ssl )
		curl_easy_setopt(h, CURLOPT_SSL_VERIFYPEER, 1L);
	else
		curl_easy_setopt(h, CURLOPT_SSL_VERIFYPEER, 0L);
	 
	curl_easy_setopt(h, CURLOPT_NOSIGNAL, 1L);	
	curl_easy_setopt(h, CURLOPT_VERBOSE, 0L);	
	curl_easy_setopt(h, CURLOPT_WRITEFUNCTION, http_recv);
    curl_easy_setopt(h, CURLOPT_WRITEDATA, out);
    curl_easy_setopt(h, CURLOPT_NOPROGRESS, TRUE);
	if ( post )
		curl_easy_setopt(h, CURLOPT_HTTPPOST, post);
		
	CURLcode res;
	res = curl_easy_perform(h);
	
	if ( res != CURLE_OK )
	{
		errno = res;
		dbg("error: !curlok == %d", res);
		curl_easy_cleanup(h);
		return -1;
	}
	
	long_t resCode;
	curl_easy_getinfo(h, CURLINFO_RESPONSE_CODE, &resCode);
    if (resCode != 200L) 
    {
        dbg("error: resCode!200 == %ld", resCode); 
        curl_easy_cleanup(h);
        return -2;
    }
	
	curl_easy_cleanup(h);
	return 0;
}
    
__private err_t http_download(file_h* fd, char_t* url, bool_t ssl)
{
	dbg("");
	iassert( NULL != fd );
	
	CURL* h;
	if ( !(h = curl_easy_init()) )
	{
		dbg("error: curl init :: %s", http_err_str());
		return -1;
	}
	
	curl_easy_setopt(h, CURLOPT_URL, url);
	
	if ( ssl )
		curl_easy_setopt(h, CURLOPT_SSL_VERIFYPEER, 1L);
	else
		curl_easy_setopt(h, CURLOPT_SSL_VERIFYPEER, 0L);
	 
	curl_easy_setopt(h, CURLOPT_VERBOSE, 0L);
	curl_easy_setopt(h, CURLOPT_WRITEFUNCTION, http_recv_file);
    curl_easy_setopt(h, CURLOPT_WRITEDATA, fd);
    curl_easy_setopt(h, CURLOPT_NOPROGRESS, TRUE);
    
	CURLcode res;
	res = curl_easy_perform(h);
	
	if ( res != CURLE_OK )
	{
		errno = res;
		dbg("error: !curlok == %d", res); 
		curl_easy_cleanup(h);
		return -1;
	}
	
	long_t resCode;
	curl_easy_getinfo(h, CURLINFO_RESPONSE_CODE, &resCode);
    if (resCode != 200L) 
    {
        dbg("error: resCode!200 == %ld", resCode); 
        curl_easy_cleanup(h);
        return -2;
    }
	
	curl_easy_cleanup(h);
	return 0;
}
/******************************************************************************************/
/******************************************************************************************/
/******************************************************************************************/
/*************************************** JSON *********************************************/
/******************************************************************************************/
/******************************************************************************************/
/******************************************************************************************/

__private inline err_t jsn_parse(jsmntok_t* tk, int_t count, tgb_s* t)
{
	dbg("");
	jsmn_parser js;
	jsmn_init(&js);
	int_t i;
	char_t* data = t->buf.data;
	
	count = jsmn_parse(&js, t->buf.data, t->buf.rem, tk, count);
	
	for( i = 0; i < count; ++i )
		data[tk[i].end] = 0;
	
	return count;
}

__private err_t jsn_skip_object(jsmntok_t** tok)
{
	jsmntok_t* tk = *tok;
	int_t n = tk->size;
	dbg("skip nobj %d %s", tk->size, tk->begin);
	++tk;
	while( n > 0 && tk->type != JSMN_END )
	{
		--n;
		if ( tk->type == JSMN_ARRAY || tk->type == JSMN_OBJECT )
		{
			dbg("\tmaster child %s", tk->begin);
			jsn_skip_object(&tk);
			dbg("\tend master child");
			continue;
		}
		else
		{
			++tk;
		}
		
		if ( tk->type == JSMN_ARRAY || tk->type == JSMN_OBJECT )
		{
			dbg("\tvalue child %s", tk->begin);
			jsn_skip_object(&tk);
			dbg("\tend value child");
		}
		else
		{
			++tk;
		}
	}
	*tok = tk;
	return 0;
}

__private jsmntok_t* jsn_get(jsmntok_t* tk, int_t n, char_t* name)
{
	dbg("search '%s' nobj %d", name, n);
	
	while( n--> 0 && tk->type != JSMN_END )
	{
		if ( !strcmp(name, tk->begin) )
		{
			dbg("\tok");
			return tk + 1;
		}
		dbg("\tno match '%s'", tk->begin);
		
		++tk;
		switch ( tk->type )
		{
			case JSMN_ARRAY:
				dbg("\tis array");
			case JSMN_OBJECT:
				dbg("\tis object");
				jsn_skip_object(&tk);
				if ( tk->type == JSMN_END )
				{
					dbg("warning: skip object to end return null");
					return NULL;
				}
			break;
			
			case JSMN_END:
				dbg("warning: not find return null");
			return NULL;
			
			default:
				dbg("\tnext tk");
				++tk;
			break;
		}
	}
	dbg("warning: end cicle return null");
	return NULL;
}

__private err_t jsn_object(jsmntok_t* tk, int_t n, char_t* name, jsmntok_t** value)
{
	dbg("");
	jsmntok_t* v = jsn_get(tk, n, name);
	if ( !v ) return -1;
	if ( v->type != JSMN_OBJECT && v->type != JSMN_ARRAY ) return -1;
	*value = v;
	return 0;
}

__private err_t jsn_bool(jsmntok_t* tk, int_t n, char_t* name, bool_t* value)
{
	dbg("");
	jsmntok_t* v = jsn_get(tk, n, name);
	if ( !v ) return -1;
	if ( JSMN_PRIMITIVE != v->type ) return -1;
	dbg("\tconvert %s -> %d", v->begin, strcmp( "false", v->begin));
	*value = strcmp( "false", v->begin); 
	return 0;
}

__private err_t jsn_int64(jsmntok_t* tk, int_t n, char_t* name, int64_t* value)
{
	dbg("");
	jsmntok_t* v = jsn_get(tk, n, name);
	if ( !v ) return -1;
	if ( JSMN_PRIMITIVE != v->type ) return -1;
	dbg("\tconvert(%p) %s, %lld", value, v->begin, strtoll(v->begin, NULL, 10));
	*value = strtoll(v->begin, NULL, 10);
	return 0;
}

__private err_t jsn_str(jsmntok_t* tk, int_t n, char_t* name, char_t** value)
{
	dbg("");
	jsmntok_t* v = jsn_get(tk, n, name);
	if ( !v ) return -1;
	if ( JSMN_STRING != v->type ) return -1;
	*value = v->begin;
	return 0;
}

__private err_t jsn_float(jsmntok_t* tk, int_t n, char_t* name, float* value)
{
	dbg("");
	jsmntok_t* v = jsn_get(tk, n, name);
	if ( !v ) return -1;
	if ( JSMN_STRING != v->type ) return -1;
	*value = strtof(v->begin, NULL);
	return 0;
}

/******************************************************************************************/
/******************************************************************************************/
/******************************************************************************************/
/********************************** TELEGRAM QUERY ****************************************/
/******************************************************************************************/
/******************************************************************************************/
/******************************************************************************************/

#define __try_dic_struct_bool(S, M)  if ( jsn_bool(tk, sz, #M, &S->M) ) return -1
#define __try_dic_struct_int64(S, M) if ( jsn_int64(tk, sz, #M, &S->M) ) return -2
#define __try_dic_struct_str(S, M)   if ( jsn_str(tk, sz, #M, &S->M) ) return -3
#define __try_dic_struct_float(S, M) if ( jsn_float(tk, sz, #M, &S->M) ) return -4

#define __try_dic_struct_obj(S, M, T, F) 	do{\
												jsmntok_t* obj;\
												if ( jsn_object(tk, sz, #M, &obj) ) { S->M = NULL; return -5;}\
												else{\
													S->M = NEW(T);\
													iassert( NULL != S->M );\
													if ( F(S->M, obj) ){\
														FREE(S->M);\
														return -1;\
													}\
												}\
											}while(0)

#define __opt_dic_struct_int64(S, M) if ( jsn_int64(tk, sz, #M, &S->M) ) S->M = 0
#define __opt_dic_struct_str(S, M)   if ( jsn_str(tk, sz, #M, &S->M) ) S->M = NULL
#define __opt_dic_struct_bool(S, M)  if ( jsn_bool(tk, sz, #M, &S->M) ) S->M = FALSE
#define __opt_dic_struct_float(S, M) if ( jsn_float(tk, sz, #M, &S->M) ) S->M = 0.0

#define __opt_dic_struct_obj(S, M, T, F) 	do{\
												jsmntok_t* obj;\
												if ( jsn_object(tk, sz, #M, &obj) ) {S->M = NULL;}\
												else{\
													S->M = NEW(T);\
													iassert( NULL != S->M );\
													if ( F(S->M, obj) ){\
														FREE(S->M);\
														return -1;\
													}\
												}\
											}while(0)
	
__private err_t tgb_query(jsmntok_t** token)
{
	dbg("");
	bool_t ok;
	jsmntok_t* root = *token;
	int_t sz = root->size;
	
	++root;
	if ( jsn_bool(root, sz, "ok", &ok) ) return -1;
	if ( ok == FALSE ) return -1;
	if ( jsn_object(root, sz, "result", token) ) return -2;
	return 0;
}	

__private err_t tgb_query_array_next(jsmntok_t** token)
{
	dbg("");
	jsn_skip_object(token);
	jsmntok_t* tk = *token;
	if ( tk->type == JSMN_END )
	{
		dbg("info: json end");
		return -1;
	}
	return 0;
}
	
__private err_t tgb_query_user(tgbUser_s* out, jsmntok_t* tk)
{
	dbg("");
	int_t sz = tk->size;
	++tk;
	
	__try_dic_struct_int64(out, id);
	__try_dic_struct_str(out, first_name);
	__opt_dic_struct_str(out, last_name);
	__opt_dic_struct_str(out, username);
	
	return 0;
}

__private err_t tgb_query_chat(tgbChat_s* out, jsmntok_t* tk)
{
	dbg("");
	int_t sz = tk->size;
	++tk;
	
	__try_dic_struct_int64(out, id);
	__try_dic_struct_str(out, type);
	__opt_dic_struct_str(out, title);
	__opt_dic_struct_str(out, username);
	__opt_dic_struct_str(out, first_name);
	__opt_dic_struct_str(out, last_name);
	__opt_dic_struct_bool(out, all_members_are_administrators);
	
	return 0;
}
/*
__private err_t tgb_query_entity(tgbEntity_s* out, jsmntok_t* tk)
{
	dbg("");
	int_t sz = tk->size;
	++tk;
	
	__try_dic_struct_str(out, type);
	__try_dic_struct_int64(out, offset);
	__try_dic_struct_int64(out, length);
	__opt_dic_struct_str(out, url);
	__opt_dic_struct_obj(out, user, tgbUser_s, tgb_query_user);
	
	return 0;
}
*/
/*
__private err_t tgb_query_photo_size(tgbPhotoSize_s* out, jsmntok_t* tk)
{
	dbg("");
	int_t sz = tk->size;
	++tk;
	
	__try_dic_struct_str(out, file_id);
	__try_dic_struct_int64(out, width);
	__try_dic_struct_int64(out, height);
	__opt_dic_struct_int64(out, file_size);
	
	return 0;
}
*/
__private err_t tgb_query_audio(tgbAudio_s* out, jsmntok_t* tk)
{
	dbg("");
	int_t sz = tk->size;
	++tk;
	
	__try_dic_struct_str(out, file_id);
	__try_dic_struct_int64(out, duration);
	__opt_dic_struct_str(out, performer);
	__opt_dic_struct_str(out, title);
	__opt_dic_struct_str(out, mime_type);
	__opt_dic_struct_int64(out, file_size);
	
	return 0;
}

__private err_t tgb_query_document(tgbDocument_s* out, jsmntok_t* tk)
{
	dbg("");
	int_t sz = tk->size;
	++tk;
	
	__try_dic_struct_str(out, file_id);
	out->thumb = NULL; 
	/*todo1 __opt_dic_struct_obj(out, thumb, tgbPhotoSize_s, tgb_query_photo_size);*/
	__opt_dic_struct_str(out, file_name);
	__opt_dic_struct_str(out, mime_type);
	__opt_dic_struct_int64(out, file_size);
	
	return 0;
}

__private err_t tgb_query_sticker(tgbSticker_s* out, jsmntok_t* tk)
{
	dbg("");
	int_t sz = tk->size;
	++tk;
	
	__try_dic_struct_str(out, file_id);
	__try_dic_struct_int64(out, width);
	__try_dic_struct_int64(out, height);
	out->thumb = NULL;
	/*todo1 __opt_dic_struct_obj(out, thumb, tgbPhotoSize_s, tgb_query_photo_size);*/
	__opt_dic_struct_str(out, emoji);
	__opt_dic_struct_int64(out, file_size);
	
	return 0;
}

__private err_t tgb_query_video(tgbVideo_s* out, jsmntok_t* tk)
{
	dbg("");
	int_t sz = tk->size;
	++tk;
	
	__try_dic_struct_str(out, file_id);
	__try_dic_struct_int64(out, width);
	__try_dic_struct_int64(out, height);
	__try_dic_struct_int64(out, duration);
	out->thumb = NULL;
	/*todo1 __opt_dic_struct_obj(out, thumb, tgbPhotoSize_s, tgb_query_photo_size);*/
	__opt_dic_struct_str(out, mime_type);
	__opt_dic_struct_int64(out, file_size);
	
	return 0;
}

__private err_t tgb_query_voice(tgbVoice_s* out, jsmntok_t* tk)
{
	dbg("");
	int_t sz = tk->size;
	++tk;
	
	__try_dic_struct_str(out, file_id);
	__try_dic_struct_int64(out, duration);
	__opt_dic_struct_str(out, mime_type);
	__opt_dic_struct_int64(out, file_size);
	
	return 0;
}

__private err_t tgb_query_contact(tgbContact_s* out, jsmntok_t* tk)
{
	dbg("");
	int_t sz = tk->size;
	++tk;
	
	__try_dic_struct_str(out, phone_number);
	__try_dic_struct_str(out, first_name);
	__opt_dic_struct_str(out, last_name);
	__opt_dic_struct_int64(out, user_id);
	
	return 0;
}

__private err_t tgb_query_location(tgbLocation_s* out, jsmntok_t* tk)
{
	dbg("");
	int_t sz = tk->size;
	++tk;
	
	__try_dic_struct_float(out, longitude);
	__try_dic_struct_float(out, latitude);
	
	return 0;
}

__private err_t tgb_query_venue(tgbVenue_s* out, jsmntok_t* tk)
{
	dbg("");
	int_t sz = tk->size;
	++tk;
	
	__try_dic_struct_obj(out, location, tgbLocation_s, tgb_query_location);
	__try_dic_struct_str(out, title);
	__try_dic_struct_str(out, address);
	__opt_dic_struct_str(out, foursquare_id);
	
	return 0;
}

__private err_t tgb_query_file(tgbFile_s* out, jsmntok_t* tk)
{
	dbg("");
	int_t sz = tk->size;
	++tk;
	
	__try_dic_struct_str(out, file_id);
	__opt_dic_struct_int64(out, file_size);
	__opt_dic_struct_str(out, file_path);
	
	return 0;
}

__private err_t tgb_query_chat_member(tgbChatMember_s* out, jsmntok_t* tk)
{
	dbg("");
	int_t sz = tk->size;
	++tk;
	
	__try_dic_struct_obj(out, user, tgbUser_s, tgb_query_user);
	__try_dic_struct_str(out, status);
	
	return 0;
}

__private err_t tgb_query_message(tgbMessage_s* out, jsmntok_t* tk)
{
	dbg("");
	int_t sz = tk->size;
	++tk;
	
	__try_dic_struct_int64(out, message_id);
	__try_dic_struct_obj(out, from, tgbUser_s, tgb_query_user);
	__try_dic_struct_int64(out, date);
	__try_dic_struct_obj(out, chat, tgbChat_s, tgb_query_chat);
	__opt_dic_struct_obj(out, forward_from, tgbUser_s, tgb_query_user);
	__opt_dic_struct_obj(out, forward_from_chat, tgbChat_s, tgb_query_chat);
	__opt_dic_struct_int64(out, forward_from_message_id);
	__opt_dic_struct_int64(out, forward_date);
	__opt_dic_struct_obj(out, reply_to_message, tgbMessage_s, tgb_query_message);
	__opt_dic_struct_int64(out, edit_date);
	__opt_dic_struct_str(out, text);
	out->entities = NULL;
	/*todo2 tgbEntity_s* entities;*/
	__opt_dic_struct_obj(out, audio, tgbAudio_s, tgb_query_audio);
	__opt_dic_struct_obj(out, document, tgbDocument_s, tgb_query_document);
	out->game = NULL;
	/*todo3 __opt_dic_struct_obj(out, game, tgbGame_s, tgb_query_game);*/
	out->photo = NULL;
	/*todo1 __opt_dic_struct_obj(out, photo, tgbPhotoSize_s, tgb_query_photo_size);*/
	__opt_dic_struct_obj(out, sticker, tgbSticker_s, tgb_query_sticker);
	__opt_dic_struct_obj(out, video, tgbVideo_s, tgb_query_video);
	__opt_dic_struct_obj(out, voice, tgbVoice_s, tgb_query_voice);
	__opt_dic_struct_str(out, caption);
	__opt_dic_struct_obj(out, contact, tgbContact_s, tgb_query_contact);
	__opt_dic_struct_obj(out, location, tgbLocation_s, tgb_query_location);
	__opt_dic_struct_obj(out, venue, tgbVenue_s, tgb_query_venue);
	__opt_dic_struct_obj(out, new_chat_member, tgbUser_s, tgb_query_user);
	__opt_dic_struct_obj(out, left_chat_member, tgbUser_s, tgb_query_user);
	__opt_dic_struct_str(out, new_chat_title);
	out->new_chat_photo = NULL;
	/*todo1 __opt_dic_struct_obj(out, new_chat_photo, tgbPhotoSize_s, tgb_query_photo_size);*/
	__opt_dic_struct_bool(out, delete_chat_photo);
	__opt_dic_struct_bool(out, group_chat_created);
	__opt_dic_struct_bool(out, supergroup_chat_created);
	__opt_dic_struct_bool(out, channel_chat_created);
	__opt_dic_struct_int64(out, migrate_to_chat_id);
	__opt_dic_struct_int64(out, migrate_from_chat_id);
	__opt_dic_struct_obj(out, pinned_message, tgbMessage_s, tgb_query_message);
		
	return 0;
}

__private err_t tgb_query_update(tgbUpdate_s* out, jsmntok_t* tk)
{
	dbg("");
	int_t sz = tk->size;
	++tk;
	
	dbg("\tupdate->update_id %p", &out->update_id);
	__try_dic_struct_int64(out, update_id);
	dbg("\tupdate->message");
	__opt_dic_struct_obj(out, message, tgbMessage_s, tgb_query_message);
	dbg("\tupdate->edited_message");
	__opt_dic_struct_obj(out, edited_message, tgbMessage_s, tgb_query_message);
	dbg("\tupdate->channel_post");
	__opt_dic_struct_obj(out, channel_post, tgbMessage_s, tgb_query_message);
	dbg("\tupdate->edited_channel_post");
	__opt_dic_struct_obj(out, edited_channel_post, tgbMessage_s, tgb_query_message);
	out->inline_query= NULL; /* todo4 */
	out->chosen_inline_result = NULL; /* todo4 */
	out->callback_query = NULL; /* todo5 */
		
	return 0;
}

/******************************************************************************************/
/******************************************************************************************/
/******************************************************************************************/
/*********************************** TELEGRAM BOT *****************************************/
/******************************************************************************************/
/******************************************************************************************/
/******************************************************************************************/

__always_inline __private void tgb_set_method(tgb_s* t, char_t* m)
{
	strcpy(t->mth, m);
}

__always_inline __private void tgb_set_fileurl(tgb_s* t, char_t* m)
{
	strcpy(t->fih, m);
}

err_t tgb_init(tgb_s* t, char_t* token)
{
	sprintf(t->baseUrl, "%s/bot%s/", TGB_URL, token);
	t->mth = t->baseUrl + strlen(t->baseUrl);
	dbg("url mth::%s", t->baseUrl);
	sprintf(t->fileUrl, "%s/file/bot%s/", TGB_URL, token);
	t->fih = t->fileUrl + strlen(t->fileUrl);
	dbg("url fih::%s", t->fileUrl);
	
	http_init();
	t->buf.data = NULL;
	return 0;
}

err_t tgb_clone(tgb_s* d, tgb_s* s)
{
	dbg("");
	*s->mth = 0;
	strcpy(d->baseUrl, s->baseUrl);
	d->mth = d->baseUrl + strlen(d->baseUrl);
	
	dbg("clone api::%s",d->baseUrl);
	
	*s->fih = 0;
	strcpy(d->fileUrl, s->fileUrl);
	d->fih = d->fileUrl + strlen(d->fileUrl);
	
	d->buf.data = 0;
	
	return 0;
}

err_t tgb_destroy(tgb_s* t)
{
	dbg("");
	FREE( t->buf.data );
	http_destroy();
	return 0;
}

err_t tgb_clean(tgb_s* t)
{
	dbg("");
	FREE(t->buf.data);
	return 0;
}

void tgb_free_document(tgbDocument_s* fr)
{
	dbg("");
	FREE(fr->thumb);
}

void tgb_free_sticker(tgbSticker_s* fr)
{
	dbg("");
	FREE(fr->thumb);
}

void tgb_free_video(tgbVideo_s* fr)
{
	dbg("");
	FREE(fr->thumb);
}

void tgb_free_venue(tgbVenue_s* fr)
{
	dbg("");
	FREE(fr->location);
}

void tgb_free_message(tgbMessage_s* fr)
{
	dbg("");
	FREE(fr->from);
	FREE(fr->chat);
	FREE(fr->forward_from);
	FREE(fr->forward_from_chat);
	if ( fr->reply_to_message )
	{
		tgb_free_message(fr->reply_to_message);
		FREE(fr->reply_to_message);
	}
	
	/* todo2 entity*/
	
	FREE(fr->audio);
	if ( fr->document )
	{
		tgb_free_document(fr->document);
		FREE(fr->document);
	}
	/*todo3 game*/
	
	if ( fr->photo ) FREE(fr->photo);
	if ( fr->sticker ) 
	{
		tgb_free_sticker(fr->sticker);
		FREE(fr->sticker);
	}
	if ( fr->video ) 
	{
		tgb_free_video(fr->video);
		FREE(fr->video);
	}
	FREE(fr->voice);
	FREE(fr->contact);
	FREE(fr->location);
	if ( fr->venue ) 
	{
		tgb_free_venue(fr->venue);
		FREE(fr->venue);
	}
	FREE(fr->new_chat_member);
	FREE(fr->left_chat_member);
	FREE(fr->new_chat_photo);
	if ( fr->pinned_message )
	{
		tgb_free_message(fr->pinned_message);
		FREE(fr->pinned_message);
	}
}

void tgb_free_update(tgbUpdate_s* fr)
{
	dbg("");
	if (fr->message) 
	{ 
		tgb_free_message(fr->message);
		FREE(fr->message);
	}
	if (fr->edited_message)
	{
		tgb_free_message(fr->edited_message);
		FREE(fr->edited_message);
	}
	if (fr->channel_post) 
	{
		tgb_free_message(fr->channel_post);
		FREE(fr->channel_post);
	}
	if (fr->edited_channel_post) 
	{
		tgb_free_message(fr->edited_channel_post);
		FREE(fr->edited_channel_post);
	}
	/* todo4 inline_query */
	/* todo4 chosen_inline_result */
	/* todo5 callback_query */
	dbg("free complete");
}

err_t tgb_get_me(tgbUser_s* out, tgb_s* t)
{
	iassert( NULL != out );
	iassert( NULL != t );
	
	tgb_set_method(t, TGB_GET_ME);
	dbg("api:: %s", t->baseUrl);
	
	err_t ret;
	
	if ( (ret = http_post( &t->buf, TGB_BUFFER_BLOCK, t->baseUrl, NULL, TRUE)) )
	{
		dbg("error: post %d", ret);
		return ret;
	}
	
	dbg("recv::'%s'", t->buf.data);
	
	jsmntok_t tk[TGB_PARSE_USER_NTOK];
	jsmntok_t* tkp = tk;
	
	if ( (ret = jsn_parse(tk, TGB_PARSE_USER_NTOK, t)) < 0 )
	{
		dbg("error: parse ret %d", ret);
		return ret;
	}
	
	if ( tgb_query(&tkp) )
	{
		dbg("error: query");
		return -1;
	}
	
	if ( tgb_query_user(out, tkp) )
	{
		dbg("error: query user");
		return -2;
	}
	
	return 0;
}

err_t tgb_confirmed_updates(tgbGetUpdate_s* opz, tgbUpdate_s* u, int_t n)
{
	dbg("confirm %lld", u[n-1].update_id + 1);
	opz->offset = u[n-1].update_id + 1;
	return 0;
}

int_t tgb_get_updates(tgbUpdate_s* out, uint_t sz, tgb_s* t, tgbGetUpdate_s* opz)
{
	iassert( NULL != out );
	iassert( NULL != t );
	iassert( NULL != opz );
	
    tgb_set_method(t, TGB_GET_UPDATES);
	dbg("api:: %s", t->baseUrl);
	
	int_t ret;
	
	http_form_init();
	http_form_add_int64_member(opz, offset);
	if ( opz->limit ) 
		http_form_add_int64_member(opz, limit);
	if ( opz->timeout ) 
		http_form_add_int64_member(opz, timeout);
	
	if ( (ret = http_post( &t->buf, TGB_BUFFER_BLOCK, t->baseUrl, HTTP_POST, TRUE)) )
	{
		dbg("error: post %d", ret);
		return ret;
	}
	
	dbg("recv::'%s'", t->buf.data);
	
	jsmntok_t tk[TGB_PARSE_UPDATE_NTOK];
	jsmntok_t* tkp = tk;
	
	if ( (ret = jsn_parse(tk, TGB_PARSE_UPDATE_NTOK, t)) < 0 )
	{
		dbg("error: parse ret %d",ret);
		return ret;
	}
	
	if ( tgb_query(&tkp) )
	{
		dbg("error: query");
		return -1;
	}
	
	
	int_t countArray = 1;
	if ( tkp->type == JSMN_ARRAY )
	{
		countArray = ( sz < (unsigned)tkp->size) ? sz : (unsigned)tkp->size;
		++tkp;
	}
	
	int_t i;
	for( i = 0; i < countArray; ++i)
	{
		dbg("cicle update[%d]", i);
		memset(&out[i], 0, sizeof(tgbUpdate_s));
		if ( tgb_query_update(&out[i], tkp) )
		{
			dbg("error: query_update retun %d", i);
			tgb_free_update(&out[i]);
			return i;
		}
		if ( tgb_query_array_next(&tkp) ) return i + 1;
	}
	
	return countArray;
}

__private err_t tgb_writer_utf8(char_t** dst, char_t** s)
{
	dbg("");
	//utf8_t* d = (utf8_t*)(*dst);
	
	while( **s == '\\' && *((*s)+1) == 'u' )
	{
		(*s) += 2;
		utf8_t u = itoutf8(strtol( *s, s, 16));
		*dst = utf8_write(*dst, u);
	}
	//(*dst) = (char_t*)d;
	return **s;
}

err_t tgb_writer(char_t** d, char_t** s)
{
	dbg("%c", **s);
	
	err_t ret = **s;
	
	switch( **s )
	{
		case '<': 
			strcpy(*d, "&lt;");
			++(*s);
			(*d) += 4;
		break;
		
		case '>':
			strcpy(*d, "&gt;");
			++(*s);
			(*d) += 4;
		break;
		
		case '&':
			strcpy(*d, "&amp;");
			++(*s);
			(*d) += 5;
		break;
		
		case '\\':
			if ( *((*s)+1) == 'u' )
				return tgb_writer_utf8(d, s);
				
		default:
			**d = **s;
			++(*d);
			++(*s);
		break;
	}
	
	return ret;
}

err_t tgb_send_message(tgb_s* t, tgbSendMessage_s* opz)
{
	iassert( NULL != t );
	iassert( NULL != opz );
	iassert( NULL != opz->text );
	
	tgb_set_method(t, TGB_SEND_MESSAGE);
	dbg("api:: %s", t->baseUrl);
	
	err_t ret;
    
	http_form_init();
	http_form_add_int64_member(opz, chat_id);
	http_form_add_member(opz, text);
	
	if ( opz->parse_mode ) 
	{
		if ( opz->parse_mode == PARSEMODE_MARKDOWN )
			http_form_add("parse_mode", "Markdown");
		else
			http_form_add("parse_mode", "HTML");
	}
	if ( opz->disable_web_page_preview != -1 ) 
		http_form_add_bool_member(opz, disable_web_page_preview);
	if ( opz->disable_notification != -1)
		http_form_add_bool_member(opz, disable_notification);
	if ( opz->reply_to_message_id) 
		http_form_add_int64_member(opz, reply_to_message_id);
	
    if ( (ret = http_post( &t->buf, TGB_BUFFER_BLOCK, t->baseUrl, HTTP_POST, TRUE)) )
	{
		dbg("error: post %d", ret);
		return ret;
	}
	
	dbg("recv::'%s'", t->buf.data);
	return strncmp("{\"ok\":true,\"result\":", t->buf.data, 20);
}

err_t tgb_forward_message(tgb_s* t, tgbForwardMessage_s* opz)
{
	iassert( NULL != t );
	iassert( NULL != opz );
	
	tgb_set_method(t, TGB_SEND_MESSAGE);
	dbg("api:: %s", t->baseUrl);
	
	err_t ret;
    
	http_form_init();
	http_form_add_int64_member(opz, chat_id);
	http_form_add_int64_member(opz, from_chat_id);
	http_form_add_int64_member(opz, message_id);
	if ( opz->disable_notification != -1 ) 
		http_form_add_bool_member(opz, disable_notification);
	
    if ( (ret = http_post( &t->buf, TGB_BUFFER_BLOCK, t->baseUrl, HTTP_POST, TRUE)) )
	{
		dbg("error: post %d", ret);
		return ret;
	}
	
	dbg("recv::'%s'", t->buf.data);
	
	return strncmp("{\"ok\":true,\"result\":", t->buf.data, 20);
}

err_t tgb_send_photo(tgb_s* t, tgbSendPhoto_s* opz)
{
	iassert( NULL != t );
	iassert( NULL != opz );
	
	tgb_set_method(t, TGB_SEND_PHOTO);
	dbg("api:: %s", t->baseUrl);
	
	err_t ret;
    
	http_form_init();
	http_form_add_int64_member(opz, chat_id);
	
	if ( file_exist(opz->photo) )
		http_form_add_file_member(opz, photo);
	else
		http_form_add_member(opz, photo);
		
    if ( opz->caption ) 
		http_form_add_member(opz, caption);
	if ( opz->reply_to_message_id) 
		http_form_add_int64_member(opz, reply_to_message_id);
	if ( opz->disable_notification != -1) 
		http_form_add_bool_member(opz, disable_notification);
	
	if ( (ret = http_post( &t->buf, TGB_BUFFER_BLOCK, t->baseUrl, HTTP_POST, TRUE)) )
	{
		dbg("error: post %d", ret);
		return ret;
	}
	
	dbg("recv::'%s'", t->buf.data);
	
	return strncmp("{\"ok\":true,\"result\":", t->buf.data, 20);
}

err_t tgb_send_audio(tgb_s* t, tgbSendAudio_s* opz)
{
	iassert( NULL != t );
	iassert( NULL != opz );
	
	tgb_set_method(t, TGB_SEND_AUDIO);
	dbg("api:: %s", t->baseUrl);
	
	err_t ret;
	
    http_form_init();
	http_form_add_int64_member(opz, chat_id);
	
	if ( file_exist(opz->audio) )
		http_form_add_file_member(opz, audio);
	else
		http_form_add_member(opz, audio);
		
    if ( opz->caption ) 
		http_form_add_member(opz, caption);
	if ( opz->disable_notification != -1 ) 
		http_form_add_bool_member(opz, disable_notification);
	if ( opz->reply_to_message_id) 
		http_form_add_int64_member(opz, reply_to_message_id);
		
		
	if ( (ret = http_post( &t->buf, TGB_BUFFER_BLOCK, t->baseUrl, HTTP_POST, TRUE)) )
	{
		dbg("error: post %d", ret);
		return ret;
	}
	
	dbg("recv::'%s'", t->buf.data);
	
	return strncmp("{\"ok\":true,\"result\":", t->buf.data, 20);
}

err_t tgb_send_document(tgb_s* t, tgbSendDocument_s* opz)
{
	iassert( NULL != t );
	iassert( NULL != opz );
	
	tgb_set_method(t, TGB_SEND_DOCUMENT);
	dbg("api:: %s", t->baseUrl);
	
	err_t ret;
	
    http_form_init();
	http_form_add_int64_member(opz, chat_id);
	
	if ( file_exist(opz->document) )
		http_form_add_file_member(opz, document);
	else
		http_form_add_member(opz, document);
		
    if ( opz->caption ) 
		http_form_add_member(opz, caption);
	if ( opz->disable_notification != -1 ) 
		http_form_add_bool_member(opz, disable_notification);
	if ( opz->reply_to_message_id) 
		http_form_add_int64_member(opz, reply_to_message_id);
		
		
	if ( (ret = http_post( &t->buf, TGB_BUFFER_BLOCK, t->baseUrl, HTTP_POST, TRUE)) )
	{
		dbg("error: post %d", ret);
		return ret;
	}
	
	dbg("recv::'%s'", t->buf.data);
	
	return strncmp("{\"ok\":true,\"result\":", t->buf.data, 20);
}

err_t tgb_send_sticker(tgb_s* t, tgbSendSticker_s* opz)
{
	iassert( NULL != t );
	iassert( NULL != opz );
	
	tgb_set_method(t, TGB_SEND_STICKER);
	dbg("api:: %s", t->baseUrl);
	
	err_t ret;
	
    http_form_init();
	http_form_add_int64_member(opz, chat_id);
	
	if ( file_exist(opz->sticker) )
		http_form_add_file_member(opz, sticker);
	else
		http_form_add_member(opz, sticker);
	
	if ( opz->disable_notification != -1 ) 
		http_form_add_bool_member(opz, disable_notification);
	if ( opz->reply_to_message_id) 
		http_form_add_int64_member(opz, reply_to_message_id);
		
		
	if ( (ret = http_post( &t->buf, TGB_BUFFER_BLOCK, t->baseUrl, HTTP_POST, TRUE)) )
	{
		dbg("error: post %d", ret);
		return ret;
	}
	
	dbg("recv::'%s'", t->buf.data);
	
	return strncmp("{\"ok\":true,\"result\":", t->buf.data, 20);
}

err_t tgb_send_video(tgb_s* t, tgbSendVideo_s* opz)
{
	iassert( NULL != t );
	iassert( NULL != opz );
	
	tgb_set_method(t, TGB_SEND_VIDEO);
	dbg("api:: %s", t->baseUrl);
	
	err_t ret;
	
    http_form_init();
	http_form_add_int64_member(opz, chat_id);
	
	if ( file_exist(opz->video) )
		http_form_add_file_member(opz, video);
	else
		http_form_add_member(opz, video);
	
	if ( opz->duration) 
		http_form_add_int64_member(opz, duration);
	if ( opz->width) 
		http_form_add_int64_member(opz, width);
	if ( opz->height) 
		http_form_add_int64_member(opz, height);
	if ( opz->disable_notification != -1 ) 
		http_form_add_bool_member(opz, disable_notification);
	if ( opz->reply_to_message_id) 
		http_form_add_int64_member(opz, reply_to_message_id);
		
		
	if ( (ret = http_post( &t->buf, TGB_BUFFER_BLOCK, t->baseUrl, HTTP_POST, TRUE)) )
	{
		dbg("error: post %d", ret);
		return ret;
	}
	
	dbg("recv::'%s'", t->buf.data);
	
	return strncmp("{\"ok\":true,\"result\":", t->buf.data, 20);
}

err_t tgb_send_voice(tgb_s* t, tgbSendVoice_s* opz)
{
	iassert( NULL != t );
	iassert( NULL != opz );
	
	tgb_set_method(t, TGB_SEND_VOICE);
	dbg("api:: %s", t->baseUrl);
	
	err_t ret;
	
    http_form_init();
	http_form_add_int64_member(opz, chat_id);
	
	if ( file_exist(opz->voice) )
		http_form_add_file_member(opz, voice);
	else
		http_form_add_member(opz, voice);
	
	if ( opz->caption ) 
		http_form_add_member(opz, caption);
	if ( opz->duration) 
		http_form_add_int64_member(opz, duration);
	if ( opz->disable_notification != -1 ) 
		http_form_add_bool_member(opz, disable_notification);
	if ( opz->reply_to_message_id) 
		http_form_add_int64_member(opz, reply_to_message_id);
		
		
	if ( (ret = http_post( &t->buf, TGB_BUFFER_BLOCK, t->baseUrl, HTTP_POST, TRUE)) )
	{
		dbg("error: post %d", ret);
		return ret;
	}
	
	dbg("recv::'%s'", t->buf.data);
	
	return strncmp("{\"ok\":true,\"result\":", t->buf.data, 20);
}

err_t tgb_send_location(tgb_s* t, tgbSendLocation_s* opz)
{
	iassert( NULL != t );
	iassert( NULL != opz );
	
	tgb_set_method(t, TGB_SEND_LOCATION);
	dbg("api:: %s", t->baseUrl);
	
	err_t ret;
	
    http_form_init();
	http_form_add_int64_member(opz, chat_id);
	http_form_add_float_member(opz, latitude);
	http_form_add_float_member(opz, longitude);
	
	if ( opz->disable_notification != -1 ) 
		http_form_add_bool_member(opz, disable_notification);
	if ( opz->reply_to_message_id) 
		http_form_add_int64_member(opz, reply_to_message_id);
		
		
	if ( (ret = http_post( &t->buf, TGB_BUFFER_BLOCK, t->baseUrl, HTTP_POST, TRUE)) )
	{
		dbg("error: post %d", ret);
		return ret;
	}
	
	dbg("recv::'%s'", t->buf.data);
	
	return strncmp("{\"ok\":true,\"result\":", t->buf.data, 20);
}

err_t tgb_send_venue(tgb_s* t, tgbSendVenue_s* opz)
{
	iassert( NULL != t );
	iassert( NULL != opz );
	
	tgb_set_method(t, TGB_SEND_VENUE);
	dbg("api:: %s", t->baseUrl);
	
	err_t ret;
	
    http_form_init();
	http_form_add_int64_member(opz, chat_id);
	http_form_add_float_member(opz, latitude);
	http_form_add_float_member(opz, longitude);
	http_form_add_member(opz, title);
	http_form_add_member(opz, address);
	
	if ( opz->foursquare_id ) 
		http_form_add_member(opz, foursquare_id);
	if ( opz->disable_notification != -1 ) 
		http_form_add_bool_member(opz, disable_notification);
	if ( opz->reply_to_message_id) 
		http_form_add_int64_member(opz, reply_to_message_id);
		
		
	if ( (ret = http_post( &t->buf, TGB_BUFFER_BLOCK, t->baseUrl, HTTP_POST, TRUE)) )
	{
		dbg("error: post %d", ret);
		return ret;
	}
	
	dbg("recv::'%s'", t->buf.data);
	
	return strncmp("{\"ok\":true,\"result\":", t->buf.data, 20);
}

err_t tgb_send_contact(tgb_s* t, tgbSendContact_s* opz)
{
	iassert( NULL != t );
	iassert( NULL != opz );
	
	tgb_set_method(t, TGB_SEND_CONTACT);
	dbg("api:: %s", t->baseUrl);
	
	err_t ret;
	
    http_form_init();
	http_form_add_int64_member(opz, chat_id);
	http_form_add_member(opz, phone_number);
	http_form_add_member(opz, first_name);
	
	if ( opz->last_name )
		http_form_add_member(opz, last_name);
	if ( opz->disable_notification != -1 ) 
		http_form_add_bool_member(opz, disable_notification);
	if ( opz->reply_to_message_id) 
		http_form_add_int64_member(opz, reply_to_message_id);
		
		
	if ( (ret = http_post( &t->buf, TGB_BUFFER_BLOCK, t->baseUrl, HTTP_POST, TRUE)) )
	{
		dbg("error: post %d", ret);
		return ret;
	}
	
	dbg("recv::'%s'", t->buf.data);
	
	return strncmp("{\"ok\":true,\"result\":", t->buf.data, 20);
}

err_t tgb_get_file(tgb_s* t, tgbGetFile_s* opz)
{
    iassert( NULL != t );
	iassert( NULL != opz );
	
	tgb_set_method(t, TGB_GET_FILE);
	dbg("api:: %s", t->baseUrl);
		
	err_t ret;
	
    http_form_init();
	http_form_add_member(opz, file_id);
		
	if ( (ret = http_post( &t->buf, TGB_BUFFER_BLOCK, t->baseUrl, HTTP_POST, TRUE)) )
	{
		dbg("error: post %d", ret);
		return ret;
	}
	
	dbg("recv::'%s'", t->buf.data);
	
	jsmntok_t tk[TGB_PARSE_FILE_NTOK];
	jsmntok_t* tkp = tk;
	
	if ( (ret = jsn_parse(tk, TGB_PARSE_FILE_NTOK, t)) < 0 )
	{
		dbg("error: parse ret %d",ret);
		return ret;
	}
	
	if ( tgb_query(&tkp) )
	{
		dbg("error: query");
		return -1;
	}
	
	tgbFile_s tf;
	if ( tgb_query_file(&tf, tkp) )
	{
		dbg("error: query_update");
		return -1;
	}
	if ( NULL == tf.file_path )
	{
		dbg("error: no receved file path");
		return -2;
	}
	
	file_h* dstF = fopen(opz->out , "w");
	if ( NULL == dstF )
	{
		dbg("error: open file");
		return -3;
	}
	
	tgb_set_fileurl(t, tf.file_path);
	
	ret = http_download( dstF, t->fileUrl, TRUE);
	fclose(dstF);
	
    if ( ret )
	{
		dbg("error: post %d", ret);
		remove(opz->out);
		return ret;
	}
	
	return 0;
}

err_t tgb_ban(tgb_s* t, tgbBan_s* opz)
{
	iassert( NULL != t );
	iassert( NULL != opz );
	
	tgb_set_method(t, TGB_BAN_MEMBER);
	dbg("api:: %s", t->baseUrl);
	
	err_t ret;
	
    http_form_init();
	http_form_add_member(opz, chat_id);
	http_form_add_member(opz, user_id);
		
	if ( (ret = http_post( &t->buf, TGB_BUFFER_BLOCK, t->baseUrl, HTTP_POST, TRUE)) )
	{
		dbg("error: post %d", ret);
		return ret;
	}
	
	dbg("recv::'%s'", t->buf.data);
	
	return 0;
}

err_t tgb_leave(tgb_s* t, tgbLeave_s* opz)
{
	iassert( NULL != t );
	iassert( NULL != opz );
	
	tgb_set_method(t, TGB_LEAVE_CHAT);
	dbg("api:: %s", t->baseUrl);
	
	err_t ret;
	
    http_form_init();
	http_form_add_member(opz, chat_id);
		
	if ( (ret = http_post( &t->buf, TGB_BUFFER_BLOCK, t->baseUrl, HTTP_POST, TRUE)) )
	{
		dbg("error: post %d", ret);
		return ret;
	}
	
	dbg("recv::'%s'", t->buf.data);
	
	return 0;
}

err_t tgb_unban(tgb_s* t, tgbUnban_s* opz)
{
	iassert( NULL != t );
	iassert( NULL != opz );
	
	tgb_set_method(t, TGB_UNBAN_MEMBER);
	dbg("api:: %s", t->baseUrl);
	
	err_t ret;
	
    http_form_init();
	http_form_add_member(opz, chat_id);
	http_form_add_member(opz, user_id);
		
	if ( (ret = http_post( &t->buf, TGB_BUFFER_BLOCK, t->baseUrl, HTTP_POST, TRUE)) )
	{
		dbg("error: post %d", ret);
		return ret;
	}
	
	dbg("recv::'%s'", t->buf.data);
	
	return 0;
}

err_t tgb_get_chat(tgbChat_s* out, tgb_s* t, tgbGetChat_s* opz)
{
	iassert( NULL != t );
	iassert( NULL != opz );
	
	tgb_set_method(t, TGB_GET_CHAT);
	dbg("api:: %s", t->baseUrl);
	
	err_t ret;
	
    http_form_init();
	http_form_add_member(opz, chat_id);
		
	if ( (ret = http_post( &t->buf, TGB_BUFFER_BLOCK, t->baseUrl, HTTP_POST, TRUE)) )
	{
		dbg("error: post %d", ret);
		return ret;
	}
	
	dbg("recv::'%s'", t->buf.data);
	
	jsmntok_t tk[TGB_PARSE_CHAT_NTOK];
	jsmntok_t* tkp = tk;
	
	if ( (ret = jsn_parse(tk, TGB_PARSE_CHAT_NTOK, t)) < 0 )
	{
		dbg("error: parse ret %d",ret);
		return ret;
	}
	
	if ( tgb_query(&tkp) )
	{
		dbg("error: query");
		return -1;
	}
	
	if ( tgb_query_chat(out, tkp) )
	{
		dbg("error: query_update");
		return -1;
	}
	
	return 0;
}

tgbChatMember_s* tgb_get_chat_administrator(uint_t* countOut, tgb_s* t, tgbGetChat_s* opz)
{
	iassert( NULL != t );
	iassert( NULL != opz );
	
	tgb_set_method(t, TGB_GET_CHAT_ADMIN);
	dbg("api:: %s", t->baseUrl);
	
	err_t ret;
	
    http_form_init();
	http_form_add_member(opz, chat_id);
		
	if ( (ret = http_post( &t->buf, TGB_BUFFER_BLOCK, t->baseUrl, HTTP_POST, TRUE)) )
	{
		dbg("error: post %d", ret);
		return NULL;
	}
	
	dbg("recv::'%s'", t->buf.data);
	
	jsmntok_t tk[TGB_PARSE_CHAT_NTOK];
	jsmntok_t* tkp = tk;
	
	if ( (ret = jsn_parse(tk, TGB_PARSE_CHAT_NTOK, t)) < 0 )
	{
		dbg("error: parse ret %d",ret);
		return NULL;
	}
	
	if ( tgb_query(&tkp) )
	{
		dbg("error: query");
		return NULL;
	}
	
	*countOut = 1;
	if ( tkp->type == JSMN_ARRAY )
	{
		*countOut = tkp->size;
		++tkp;
	}
	tgbChatMember_s* out = NEWS(tgbChatMember_s, *countOut);
	
	uint_t i;
	for( i = 0; i < *countOut; ++i )
	{
		if ( tgb_query_chat_member(&out[i], tkp) )
		{
			dbg("error: query_chat_member");
			*countOut = i;
			return out;
		}
		if ( tgb_query_array_next(&tkp) ) break;
	}
	
	return out;
}

err_t tgb_get_chat_member(tgbChatMember_s* out, tgb_s* t, tgbGetChatMember_s* opz)
{
	iassert( NULL != t );
	iassert( NULL != opz );
	
	tgb_set_method(t, TGB_GET_CHAT_MEMBER);
	dbg("api:: %s", t->baseUrl);
	
	err_t ret;
	
    http_form_init();
	http_form_add_member(opz, chat_id);
	http_form_add_member(opz, user_id);
		
	if ( (ret = http_post( &t->buf, TGB_BUFFER_BLOCK, t->baseUrl, HTTP_POST, TRUE)) )
	{
		dbg("error: post %d", ret);
		return ret;
	}
	
	dbg("recv::'%s'", t->buf.data);
	
	jsmntok_t tk[TGB_PARSE_CHAT_NTOK];
	jsmntok_t* tkp = tk;
	
	if ( (ret = jsn_parse(tk, TGB_PARSE_CHAT_NTOK, t)) < 0 )
	{
		dbg("error: parse ret %d",ret);
		return ret;
	}
	
	if ( tgb_query(&tkp) )
	{
		dbg("error: query");
		return -1;
	}
	
	if ( tgb_query_chat_member(out, tkp) )
	{
		dbg("error: query_update");
		return -1;
	}
	
	return 0;
}

/******************************************************************************************/
/******************************************************************************************/
/******************************************************************************************/
/*************************************** ASYNC ********************************************/
/******************************************************************************************/
/******************************************************************************************/
/******************************************************************************************/

__private void* tgb_injection(void* arg)
{
	THREAD_START(arg, tgbThread_s*, tthr);
	
	dbg("start thread");
	
	while( !thr_chkrequestend(_this) )
	{
		dbg("release semaphore");
		thr_semaphore_post(tthr->semcount);
		dbg("thread Wait");
		thr_suspendme(_this);
				
		dbg("call action");
		tthr->fnc( &tthr->bot, &tthr->update);
		
		dbg("free update:: %lld", (tthr->update.message && tthr->update.message->message_id) ? tthr->update.message->message_id : -1LL);
		tgb_free_update(&tthr->update);
		memset(&tthr->update, 0, sizeof(tgbUpdate_s));
		
		if ( 0 == thr_resource_release(tthr->res) )
		{
			dbg("release resources %p", tthr->rsptr);
			FREE(tthr->rsptr);
			thr_resource_complete(tthr->res);
			thr_resource_destroy(tthr->res);
			FREE(tthr->res);
		}
		
		dbg("release curl buffer");
		FREE(tthr->bot.buf.data);
	}
	
	THREAD_END(0);
}

err_t tgb_async_init(tgbAsync_s* ta, uint_t count)
{
	dbg("");
	ta->count = count;
	
	ta->tgbthr = NEWS(tgbThread_s, count);
	iassert( NULL != ta->tgbthr );
	
	thr_semaphore_init(&ta->semcount, 0);
	ta->cres = 0;
	
	uint_t i;
	for( i = 0; i < count; ++i)
	{
		dbg("init thread %u", i);
		if ( thr_init(&ta->tgbthr[i].thr, tgb_injection, 0, 0, 0) ) goto ON_FAIL;
		ta->tgbthr[i].fnc = NULL;
		ta->tgbthr[i].semcount = &ta->semcount;
		if ( thr_run(&ta->tgbthr[i].thr, &ta->tgbthr[i]) ) goto ON_FAIL;
	}
		
	return 0;
	
	ON_FAIL:
	dbg("error: init thread");
	while( i-- > 0 )
	{
		thr_destroy(&ta->tgbthr[i].thr, 0);
	}
	FREE(ta->tgbthr);
	thr_semaphore_destroy(&ta->semcount);
	return -1;
}

err_t tgb_async_action(tgb_s* t, tgbAsync_s* ta, tgbAction_f fnc, tgbUpdate_s* up)
{
	dbg("action libre %u", thr_semaphore_get(&ta->semcount));
	
	thr_semaphore_wait(&ta->semcount);
	
	if ( 0 == ta->cres )
	{
		ta->cres = NEW(gresource_s);
		iassert( NULL != ta->cres );
		thr_resource_init(ta->cres);
		thr_resource_employ(ta->cres);
	}
	
	uint_t i;
	do
	{
		dbg("never fail");
		for(i = 0; i < ta->count && ta->tgbthr[i].thr.stato != T_PAUSE; ++i);
	}while( i >= ta->count );
	
	dbg("use thr %u", i);
	
	ta->tgbthr[i].update = *up;
	ta->tgbthr[i].fnc = fnc;
	ta->tgbthr[i].res = ta->cres;
	ta->tgbthr[i].rsptr = t->buf.data;
	tgb_clone(&ta->tgbthr[i].bot, t);
	thr_resource_employ(ta->cres);
	
	dbg("enable thread");
	thr_resume(&ta->tgbthr[i].thr);
	
	return 0;
}

err_t tgb_async_end(tgb_s* t, tgbAsync_s* ta)
{
	dbg("");
	if ( !ta->cres ) return 0;
	
	if ( thr_resource_release(ta->cres) )
	{
		dbg("resources destroy another thread %p", t->buf.data);
		t->buf.data = 0;
		ta->cres = 0;
		return 0;
	}
	
	dbg("reuse resources %p", t->buf.data);
	thr_resource_complete(ta->cres);
	thr_resource_destroy(ta->cres);
	FREE(ta->cres);
	
	return 0;
}

err_t tgb_async_destroy(tgbAsync_s* ta)
{
	dbg("");
	
	uint_t i;
	for( i = 0; i < ta->count; ++i)
	{
		thr_destroy(&ta->tgbthr[i].thr, TGB_THREAD_TIMEOUT);
		/*resources?*/
		/*thread buf data? if ( ta->tgbthr[i].bot.buf.data ) free(ta->tgbthr[i].bot.buf.data);*/
	}
	thr_semaphore_destroy(&ta->semcount);
	return 0;
}








