#include <tgb/terminale.h>
#include <tgb/api.h>

#define MAX_MESSAGE 30
#define MAX_TIMEOUT 600

#define BOT_TOKEN "YOUR TOKEN HERE" 

int main()
{
	tgb_s bot;
	tgbGetUpdate_s gu = { 0, MAX_MESSAGE, MAX_TIMEOUT};
	tgbUpdate_s update[MAX_MESSAGE];
	int_t countUpdate;
	
	fputs("init bot...\n", stderr);
	tgb_init(&bot, BOT_TOKEN);
	
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
			if ( NULL == update[i].message ) continue;
			if ( NULL == update[i].message->text ) continue;
			
			if ( 0 == strcmp(update[i].message->text, "/start") )
			{
				tgbSendMessage_s msg = { update[i].message->chat->id , 
										 "Hello World!", 
										 PARSEMODE_MARKDOWN, 
										 UNUSED_PREVIEW, UNUSED_NOTIFICATION, UNUSED_REPLY};
				
				tgb_send_message(&bot, &msg);
			}
			tgb_free_update(&update[i]);
		}
	}
	
	tgb_destroy(&bot);	
	
    return 0;
}
