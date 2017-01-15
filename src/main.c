#ifdef _APP

#include <tgb/terminale.h>
#include <tgb/api.h>

#define MAX_MESSAGE 10
#define MAX_THREADS 4
#define MAX_TIMEOUT 600

#define BOT_TOKEN "" 

err_t bot_action(tgb_s* bot, tgbUpdate_s* up)
{
	iassert( NULL != up );
	iassert( NULL != up->message );
	iassert( NULL != up->message->text );
	
	printf("thread compare command {%lld}[%s]\n", up->message->message_id, up->message->text);
	
	//printf("delay\n");
	//thr_sleep(10);
	//printf("end delay\n");
	
	if ( !strcmp( up->message->text, "/help" ) )
	{
		puts("command help");
		tgbSendMessage_s msg = { up->message->chat->id , 
								 "This is a simple test bot\n"
								 "/bold <text> _rewrite text to bold_", 
							     PARSEMODE_MARKDOWN, 
							     UNUSED_PREVIEW, UNUSED_NOTIFICATION, UNUSED_REPLY};
							    
		tgb_send_message( bot, &msg);
		
		return 0;
	}
	
	if ( !strncmp( up->message->text, "/bold", 5) )
	{
		puts("command bold");
		
		tgbSendMessage_s msg = { up->message->chat->id , 
								 NULL, 
							     PARSEMODE_MARKDOWN, 
							     UNUSED_PREVIEW, UNUSED_NOTIFICATION, UNUSED_REPLY};
							     
		
		if ( strlen(up->message->text) > 4090 )
		{
			puts("command too long\n");
			msg.text = "_Message too long_";
			tgb_send_message( bot, &msg);
			return 0;
		}
		
		char_t* m = up->message->text + 5;
		if ( *m != ' ' ) 
		{
			puts("command format error\n");
			msg.text = "_Message format error_";
			tgb_send_message( bot, &msg);
			return 0;
		}
		
		while( *m == ' ') ++m; 
		
		char_t reply[4096];
		char_t* d = reply;
		
		*d++ = '*';
		while( (*d++ = *m++) );
		*(d-1) = '*';
		*d = 0;
		
		msg.text = reply;
		printf("--> %lld [%s]\n\n", msg.chat_id, msg.text);
		fflush(stdout);
		
		tgb_send_message( bot, &msg);
		
		return 0;
	}
	
	puts("command not available\n");
	
	return 0;
}


int main()
{
	tgb_s bot;
	tgbGetUpdate_s gu = { 0, MAX_MESSAGE, MAX_TIMEOUT};
	tgbUpdate_s update[MAX_MESSAGE];
	tgbAsync_s async;
	int_t countUpdate;
	
	puts("init bot...");
	tgb_init(&bot, BOT_TOKEN);
	if ( tgb_async_init(&async, MAX_THREADS) )
	{
		puts("error: create thread");
		return -1;
	}
	
	while(1)
	{
		printf("wait data\n");
		countUpdate = tgb_get_updates( update, MAX_MESSAGE, &bot, &gu);
		if ( countUpdate <= 0 )
		{
			printf("no data:%d\n", countUpdate);
			continue;
		}
		
		printf("get %d message\n", countUpdate);
		tgb_confirmed_updates( &gu, update, countUpdate);
		
		int i;
		for( i = 0; i < countUpdate; ++i )
		{
			if ( NULL == update[i].message ) continue;
			if ( NULL == update[i].message->text ) continue;
			
			if( *update[i].message->text == '/')
			{
				printf("execute command[%lld] '%s'\n", update[i].message->message_id, update[i].message->text);
				tgb_async_action( &bot, &async, bot_action, &update[i]);
			}
			else
			{
				dbg("free update:: %lld", (update[i].message && update[i].message->message_id) ? update[i].message->message_id : -1LL);
				tgb_free_update(&update[i]);
			}
		}
		
		tgb_async_end(&bot, &async);
		
		#ifdef _PROF
			break;
		#endif
	}
	
	tgb_destroy(&bot);	
	
    return 0;
}

#endif /*_APP*/
