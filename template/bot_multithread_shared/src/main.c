#include <tgb/terminale.h>
#include <tgb/api.h>

#define MAX_MESSAGE 30
#define MAX_THREADS 4
#define MAX_TIMEOUT 600

#define BOT_TOKEN "YOUR TOKEN HERE"

err_t bot_action(tgb_s* bot, tgbUpdate_s* up)
{
	if ( !strcmp( up->message->text, "/help" ) )
	{
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
		tgbSendMessage_s msg = { up->message->chat->id , 
								 NULL, 
							     PARSEMODE_MARKDOWN, 
							     UNUSED_PREVIEW, UNUSED_NOTIFICATION, UNUSED_REPLY};
							     
		
		if ( strlen(up->message->text) > 4090 )
		{
			msg.text = "_Message too long_";
			tgb_send_message( bot, &msg);
			return 0;
		}
		
		char_t* m = up->message->text + 5;
		if ( *m != ' ' ) 
		{
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
		tgb_send_message( bot, &msg);
		
		return 0;
	}
	
	return 0;
}


int main()
{
	tgb_s bot;
	tgbGetUpdate_s gu = { 0, MAX_MESSAGE, MAX_TIMEOUT};
	tgbUpdate_s update[MAX_MESSAGE];
	tgbAsync_s async;
	int_t countUpdate;
		
	tgb_init(&bot, BOT_TOKEN);
	if ( tgb_async_init(&async, MAX_THREADS) )
	{
		fputs("error: create thread", stderr);
		return -1;
	}
	
	while(1)
	{
		fputs("wait data\n", stderr);
		countUpdate = tgb_get_updates( update, MAX_MESSAGE, &bot, &gu);
		if ( countUpdate <= 0 )
		{
			fprintf(stderr, "no data:%d\n", countUpdate);
			continue;
		}
		
		fprintf(stderr, "get %d message\n", countUpdate);
		tgb_confirmed_updates( &gu, update, countUpdate);
		
		int i;
		for( i = 0; i < countUpdate; ++i )
		{
			if( NULL == update[i].message ) continue;
			if( NULL == update[i].message->text ) continue;
			
			if( *update[i].message->text == '/')
			{
				tgb_async_action( &bot, &async, bot_action, &update[i]);
			}
			else
			{
				tgb_free_update(&update[i]);
			}
		}
		
		tgb_async_end(&bot, &async);
	}
	
	tgb_destroy(&bot);	
	
    return 0;
}
