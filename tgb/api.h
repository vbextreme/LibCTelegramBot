#ifndef _TGBAPI_H_INCLUDE_
#define _TGBAPI_H_INCLUDE_

#include <tgb/stdextra.h>
#include <tgb/easythread.h>

/*todo reply_markup in send struct*/
/*todo se si toglie il continue da next_object va in errore free_message*/

#define TGB_MAX_FILE        (20L*(1024L*1024L))
#define TGB_MAX_TEXT 		4096
#define TGB_MAX_CAPTION 	200
#define TGB_MAX_USERNAME 	32
#define UNUSED_PREVIEW 		-1
#define UNUSED_NOTIFICATION -1
#define UNUSED_REPLY		0
#define TGB_THREAD_TIMEOUT  3000
#define TGB_THREAD_RELAX    0.25

#define TGB_BUFFER_BLOCK	1024
#define TGB_UPDATE_TIME     1000000
#define TGB_UPDATE_LIMIT    1
#define TGB_UPDATE_REQUEST  10
#define TGB_URL			    "https://api.telegram.org"
#define TGB_URL_MAX 		256
#define TGB_GET_ME          "getMe"
#define TGB_GET_UPDATES     "getUpdates"
#define TGB_SEND_MESSAGE    "sendMessage"
#define TGB_FORWARD_MESSAGE "forwardMessage"
#define TGB_SEND_PHOTO      "sendPhoto"
#define TGB_SEND_AUDIO      "sendAudio"
#define TGB_SEND_DOCUMENT   "sendDocument"
#define TGB_SEND_VIDEO      "sendVideo"
#define TGB_SEND_STICKER    "sendSticker"
#define TGB_SEND_VOICE      "sendVoice"
#define TGB_SEND_LOCATION   "sendLocation"
#define TGB_SEND_VENUE      "sendVenue"
#define TGB_SEND_CONTACT    "sendContact"
#define TGB_SEND_CHATACTION "sendChatAction"
#define TGB_GET_USERPHOTOS  "getUserProfilePhotos"
#define TGB_GET_FILE        "getFile"
#define TGB_BAN_MEMBER      "kickChatMember"
#define TGB_LEAVE_CHAT      "leaveChat"
#define TGB_UNBAN_MEMBER	"unbanChatMember"
#define TGB_GET_CHAT        "getChat"
#define TGB_GET_CHAT_ADMIN  "getChatAdministrators"
#define TGB_GET_CHAT_COUNT  "getChatMembersCount"
#define TGB_GET_CHAT_MEMBER "getChatMember"

typedef struct httpBuffer
{
	char_t* data;
	char_t* pdata;
	uint_t size;
	uint_t rem;
	uint_t block;
}httpBuffer_s;

typedef struct curl_httppost curlHttpPost_s;

typedef struct tgb
{
	char_t baseUrl[TGB_URL_MAX];
	char_t fileUrl[TGB_URL_MAX];
	char_t* mth;
	char_t* fih;
	httpBuffer_s buf;
}tgb_s;

typedef struct tgbUser
{
	int64_t id;
	char_t* first_name;
	char_t* last_name;
	char_t* username;
}tgbUser_s;

typedef struct tgbChat
{
	int64_t id;
	char_t* type;
	char_t* title;
	char_t* username;
	char_t* first_name;
	char_t* last_name;
	bool_t  all_members_are_administrators;
}tgbChat_s;

typedef struct tgbEntity
{
	char_t* type;
	int64_t offset;
	int64_t length;
	char_t* url;
	tgbUser_s* user;
}tgbEntity_s;

typedef struct tgbPhotoSize
{
	char_t* file_id;
	int64_t width;
	int64_t height;
	int64_t file_size;
}tgbPhotoSize_s;

typedef struct tgbAudio
{
	char_t* file_id;
	int64_t duration;
	char_t* performer;
	char_t* title;
	char_t* mime_type;
	int64_t file_size;
}tgbAudio_s;

typedef struct tgbDocument
{
	char_t* file_id;
	tgbPhotoSize_s* thumb;
	char_t* file_name;
	char_t* mime_type;
	int64_t file_size;
}tgbDocument_s;

typedef struct tgbSticker
{
	char_t* file_id;
	int64_t width;
	int64_t height;
	tgbPhotoSize_s* thumb;
	char_t* emoji;
	int64_t file_size;
}tgbSticker_s;

typedef struct tgbVideo
{
	char_t* file_id;
	int64_t width;
	int64_t height;
	int64_t duration;
	tgbPhotoSize_s* thumb;
	char_t* mime_type;
	int64_t file_size;
}tgbVideo_s;

typedef struct tgbVoice
{
	char_t* file_id;
	int64_t duration;
	char_t* mime_type;
	int64_t file_size;
}tgbVoice_s;

typedef struct tgbContact
{
	char_t* phone_number;
	char_t* first_name;
	char_t* last_name;
	int64_t user_id;
}tgbContact_s;

typedef struct tgbLocation
{
	float longitude;
	float latitude;
}tgbLocation_s;

typedef struct tgbVenue
{
	tgbLocation_s* location;
	char_t* title;
	char_t* address;
	char_t* foursquare_id;
}tgbVenue_s;

/*todo*/
typedef struct tgbUserProfilePhotos
{
	int64_t totalCount;
	tgbPhotoSize_s** photos;
}tgbUserProfilePhotos_s;

typedef struct tgbFile
{
	char_t* file_id;
	int64_t file_size;
	char_t* file_path;
}tgbFile_s;

typedef struct tgbKeyboardButton
{
	char_t* text;
	bool_t request_contact;
	bool_t request_location;
}tgbKeyboardButton_s;

typedef struct tgbReplyKeyboardMarkup
{
	tgbKeyboardButton_s** keyboard;
	bool_t resize_keyboard;
	bool_t one_time_keyboard;
	bool_t selective;
}tgbReplyKeyboardMarkup_s;

typedef struct tgbReplyKeyboardRemove
{
	bool_t remove_keyboard;
	bool_t selective;
}tgbReplyKeyboardRemove_s;

typedef struct tgbInlineKeyboardButton
{
	char_t* text;
	char_t* url;
	char_t* callback_data;
	char_t* switch_inline_query;
	char_t* switch_inline_query_current_chat;
	/*todo callback_game*/ 
}tgbInlineKeyboardButton_s;

typedef struct tgbInlineKeyboardMarkup
{
	tgbInlineKeyboardButton_s* inline_keyboard;
}tgbInlineKeyboardMarkup_s;

/*todo CallbackQuery*/

typedef struct tgbForceReply
{
	bool_t force_reply;
	bool_t selective;
}tgbForceReply_s;

typedef struct tgbChatMember
{
	tgbUser_s* user;
	char_t* status;
}tgbChatMember_s;

typedef struct tgbResponseParameters
{
	int64_t migrate_chat_id;
	int64_t retry_after;
}tgbResponseParameters_s;

typedef struct tgbAnimation
{
	char_t* fileID;
	tgbPhotoSize_s* thumb;
	char_t* fileName;
	char_t* mimeType;
	int64_t fileSize;
}tgbAnimation_s;

typedef struct tgbGame
{
	char_t* title;
	char_t* description;
	tgbPhotoSize_s* photo;
	char_t* text;
	tgbEntity_s* textEntities;
	tgbAnimation_s animation;
}tgbGame_s;

typedef struct tgbMessage
{
	int64_t message_id;
	tgbUser_s* from;
	int64_t date;
	tgbChat_s* chat;
	tgbUser_s* forward_from;
	tgbChat_s* forward_from_chat;
	int64_t forward_from_message_id;
	int64_t forward_date;
	struct tgbMessage* reply_to_message;
	int64_t edit_date;
	char_t* text;
	tgbEntity_s* entities;
	tgbAudio_s*	audio;
	tgbDocument_s* document;
    tgbGame_s* game;
	tgbPhotoSize_s* photo;
	tgbSticker_s* sticker;
	tgbVideo_s* video;
	tgbVoice_s* voice;
	char_t* caption;
	tgbContact_s* contact;
	tgbLocation_s* location;
 	tgbVenue_s* venue;
	tgbUser_s* new_chat_member;
	tgbUser_s* left_chat_member;
	char_t* new_chat_title;
	tgbPhotoSize_s* new_chat_photo;
	bool_t delete_chat_photo;
	bool_t group_chat_created;
	bool_t supergroup_chat_created;
	bool_t channel_chat_created;
	int64_t migrate_to_chat_id;
	int64_t migrate_from_chat_id;
	struct tgbMessage* pinned_message;
}tgbMessage_s;

typedef struct tgbCallbackQuery
{
	char_t* id;
	tgbUser_s from;
	tgbMessage_s* message;
	char_t* inlineMessageID;
	char_t* chatInstance;
	char_t* data;
	char_t* gameShortName;
}tgbCallbackQuery_s;

typedef struct tgbInlineQuery
{
	char_t* id;
	tgbUser_s* from;
	tgbLocation_s* location;
	char_t* query;
	char_t* offset;
}tgbInlineQuery_s;

typedef struct tgbChosenInlineResult
{
	char_t* result_id;
	tgbUser_s* from;
	tgbLocation_s* location;
	char_t* inline_message_id;
	char_t* query;
}tgbChosenInlineResult_s;

typedef struct tgbUpdate
{
	int64_t update_id;
	tgbMessage_s* message;
	tgbMessage_s* edited_message;
	tgbMessage_s* channel_post;
	tgbMessage_s* edited_channel_post;
	tgbInlineQuery_s* inline_query;
	tgbChosenInlineResult_s* chosen_inline_result;
	tgbCallbackQuery_s* callback_query;
}tgbUpdate_s;

typedef struct tgbGetUpdate
{
	int64_t offset;
	int64_t limit;
	int64_t timeout;
}tgbGetUpdate_s;

typedef enum { PARSEMODE_NONE, PARSEMODE_MARKDOWN, PARSEMODE_HTML, PARSEMODE_COUNT } tgbParseMode_e;

typedef struct tgbSendMessage
{
	int64_t chat_id;
	char_t* text;
	tgbParseMode_e parse_mode;
	int_t disable_web_page_preview;
	int_t disable_notification;
	int64_t reply_to_message_id;
}tgbSendMessage_s;

typedef struct tgbForwardMessage
{
	int64_t chat_id;
	int64_t from_chat_id;
	int_t disable_notification;
	int64_t message_id;
}tgbForwardMessage_s;

typedef struct tgbSendPhoto
{
	int64_t chat_id;
	char_t* photo;
	char_t* caption;
	int_t disable_notification;
	int64_t reply_to_message_id;
}tgbSendPhoto_s;

typedef struct tgbSendAudio
{
	int64_t chat_id;
	char_t* audio;
	char_t* caption;
	int64_t duration;
	char_t* performer;
	char_t* title;
	int_t disable_notification;
	int64_t reply_to_message_id;
}tgbSendAudio_s;

typedef struct tgbSendDocument
{
	int64_t chat_id;
	char_t* document;
	char_t* caption;
	int_t disable_notification;
	int64_t reply_to_message_id;
}tgbSendDocument_s;

typedef struct tgbSendSticker
{
	int64_t chat_id;
	char_t* sticker;
	int_t disable_notification;
	int64_t reply_to_message_id;
}tgbSendSticker_s;

typedef struct tgbSendVideo
{
	int64_t chat_id;
	char_t* video;
	int64_t duration;
	int64_t width;
	int64_t height;
	int_t disable_notification;
	int64_t reply_to_message_id;
}tgbSendVideo_s;

typedef struct tgbSendVoice
{
	int64_t chat_id;
	char_t* voice;
	char_t* caption;
	int64_t duration;
	int_t disable_notification;
	int64_t reply_to_message_id;
}tgbSendVoice_s;

typedef struct tgbSendLocation
{
	int64_t chat_id;
	float latitude;
	float longitude;
	int_t disable_notification;
	int64_t reply_to_message_id;
}tgbSendLocation_s;

typedef struct tgbSendVenue
{
	int64_t chat_id;
	float latitude;
	float longitude;
	char_t* title;
	char_t* address;
	char_t* foursquare_id;
	int_t disable_notification;
	int64_t reply_to_message_id;
}tgbSendVenue_s;

typedef struct tgbSendContact
{
	int64_t chat_id;
	char_t* phone_number;
	char_t* first_name;
	char_t* last_name;
	int_t disable_notification;
	int64_t reply_to_message_id;
}tgbSendContact_s;

typedef struct tgbGetFile
{
	char_t* file_id;
	char_t* out;
}tgbGetFile_s;

typedef struct tgbBan
{
	char_t* chat_id;
	char_t* user_id;
}tgbBan_s;

typedef struct tgbLeave
{
	char_t* chat_id;
}tgbLeave_s;

typedef struct tgbUnban
{
	char_t* chat_id;
	char_t* user_id;
}tgbUnban_s;

typedef struct tgbGetChat
{
	char_t* chat_id;
}tgbGetChat_s;

typedef struct tgbGetChatMember
{
	char_t* chat_id;
	char_t* user_id;
}tgbGetChatMember_s;

typedef err_t(*tgbAction_f)(tgb_s* ,tgbUpdate_s*);

typedef struct tgbThread
{
	thread_s thr;
	tgbAction_f fnc;
	tgb_s bot;
	tgbUpdate_s update;
	//semaphore_h await;
	semaphore_h* semcount;
	gresource_s* res;
	void* rsptr;
}tgbThread_s;

typedef struct tgbAsync
{
	tgbThread_s* tgbthr;
	gresource_s* cres;
	semaphore_h semcount;
	uint_t count;
}tgbAsync_s;

err_t file_exist(char_t* name);
char_t* i64toa(char_t* d, int64_t s);
err_t tgb_init(tgb_s* t, char_t* token);
err_t tgb_clone(tgb_s* d, tgb_s* s);
err_t tgb_destroy(tgb_s* t);
err_t tgb_clean(tgb_s* t);
void tgb_free_update(tgbUpdate_s* fr);
err_t tgb_get_me(tgbUser_s* out, tgb_s* t);
err_t tgb_confirmed_updates(tgbGetUpdate_s* opz, tgbUpdate_s* u, int_t n);
int_t tgb_get_updates(tgbUpdate_s* out, uint_t sz, tgb_s* t, tgbGetUpdate_s* opz);
err_t tgb_writer(char_t** d, char_t** s);
err_t tgb_send_message(tgb_s* t, tgbSendMessage_s* opz);
err_t tgb_forward_message(tgb_s* t, tgbForwardMessage_s* opz);
err_t tgb_send_photo(tgb_s* t, tgbSendPhoto_s* opz);
err_t tgb_send_document(tgb_s* t, tgbSendDocument_s* opz);
err_t tgb_send_sticker(tgb_s* t, tgbSendSticker_s* opz);
err_t tgb_send_video(tgb_s* t, tgbSendVideo_s* opz);
err_t tgb_send_venue(tgb_s* t, tgbSendVenue_s* opz);
err_t tgb_send_contact(tgb_s* t, tgbSendContact_s* opz);
/*err_t tgb_send_chat_action(tgb_s* t, _s* opz);*/
/*err_t tgb_get_user_profile_photo(tgb_s* t, _s* opz);*/
err_t tgb_get_file(tgb_s* t, tgbGetFile_s* opz);
err_t tgb_ban(tgb_s* t, tgbBan_s* opz);
err_t tgb_leave(tgb_s* t, tgbLeave_s* opz);
err_t tgb_unban(tgb_s* t, tgbUnban_s* opz);
err_t tgb_get_chat(tgbChat_s* out, tgb_s* t, tgbGetChat_s* opz);
tgbChatMember_s* tgb_get_chat_administrator(uint_t* countOut, tgb_s* t, tgbGetChat_s* opz);
err_t tgb_get_chat_member(tgbChatMember_s* out, tgb_s* t, tgbGetChatMember_s* opz);
err_t tgb_async_init(tgbAsync_s* ta, uint_t count);
err_t tgb_async_action(tgb_s* t, tgbAsync_s* ta, tgbAction_f fnc, tgbUpdate_s* up);
err_t tgb_async_end(tgb_s* t, tgbAsync_s* ta);
err_t tgb_async_destroy(tgbAsync_s* ta);

#endif
