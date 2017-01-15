# Lib C Telegram Bot

simple library that allows you to create a bot telegram with the language C<br/>
The library exposes methods to work in multithreading
special thanks for [jsmn](https://github.com/zserge/jsmn)

## Getting Started

These instructions will get you a copy of the project up and running on your local machine for development and testing purposes. See deployment for notes on how to deploy the project on a live system.

### Prerequisites

This library requires [CURL](https://curl.haxx.se/)<br/>
Clone project

```
$ git clone https://github.com/vbextreme/LibCTelegramBot.git
$ cd LibCTelegramBot
```

### Build

The makefile exposes several tools to simplify building and software development, at end of the building will result library libtgb.so in the directory "build_shared"

```
$ make
```

### Developer

developers can create an executable instead of the library, building the "main.c" like this will test faster changes to the library itself.

Build application
```
$ make tst
```

Build with debug
```
$ make dbg
```

Build and profiling with gprof
```
$ make prof
```

Build and profiling with valgrind
```
$ make valgrind
$ make valgrind.view
```

### Installing

The installation will copy the headers in the directory "/usr/include/tgb" and the library "/usr/lib"<br/>
you can edit "configSHARED.mk" for change destination directory and other library flags.

```
$ sudo make install
```

### Uninstall

always with makefile

```
$ sudo make uninstall
```

## Create your bot

### Use Template

simple way to create a bot is to use the template.<br/>
just copy, rename the directory and write your bot.

```
$ cd template
$ cp -r ./bot_singlethread_shared ./mybot
$ cd ./mybot
```

add your code in "src/main.c" and build with

```
$ make
```

you can install your bot 

```
$ make install
```

for edit option compiler need change file "configAPP.mk"

## How Work

### Single Thread

* Init libtgb
* in loop call api tgb_get_update()
* get structure tgbUpdate
* execute your bot
* loop

### Multi Thread

* Init libtgb
* Init Thread
* in loop call api tgb_get_update()
* get structure tgbUpdate
* call action in other thread
* loop

## Documentation

Telegram send json message with very large number of optional variable.<br/>
for this reason all the contents of the structure, except for the integer values, ​​are pointers they will be NULL in case of missing parameter.

### API

err_t type return 0 for success or negative number for error.<br/>
<br/>
check a file exist in your local directory
```
err_t file_exist(char_t* name);
```

convert int64_t to char string
```
char_t* i64toa(char_t* d, int64_t s);
```

init a telegram lib
```
err_t tgb_init(tgb_s* t, char_t* token);
```

release all resource of telegram lib
```
err_t tgb_destroy(tgb_s* t);
```

free data in tgbUpdate structure, you need call only in main thread, when you pass this structure in the action you not need to free.<br/>
```
void tgb_free_update(tgbUpdate_s* fr);
```

fill structure tgbUser with reply of telegram method getMe
```
err_t tgb_get_me(tgbUser_s* out, tgb_s* t);
```

each time you receive one or more messages must be confirmed receiving them, otherwise they will be sent back
```
err_t tgb_confirmed_updates(tgbGetUpdate_s* opz, tgbUpdate_s* u, int_t n);
```

fill vector structure tgbUpdate with reply of telegram method getUpdates
```
int_t tgb_get_updates(tgbUpdate_s* out, uint_t sz, tgb_s* t, tgbGetUpdate_s* opz);
```

telegram use a varius escape sequence, you can copy "s" into "d" with _writer, "d" and "s" are autoincrement, return char copied
```
err_t tgb_writer(char_t** d, char_t** s);
```

telegram method sendMessage
```
err_t tgb_send_message(tgb_s* t, tgbSendMessage_s* opz);
```

telegram method forwardMessage
```
err_t tgb_forward_message(tgb_s* t, tgbForwardMessage_s* opz);
```

telegram method sendPhoto
```
err_t tgb_send_photo(tgb_s* t, tgbSendPhoto_s* opz);
```

telegram method sendDocument
```
err_t tgb_send_document(tgb_s* t, tgbSendDocument_s* opz);
```

telegram method sendSticker
```
err_t tgb_send_sticker(tgb_s* t, tgbSendSticker_s* opz);
```

telegram method sendVideo
```
err_t tgb_send_video(tgb_s* t, tgbSendVideo_s* opz);
```

telegram method sendVenue
```
err_t tgb_send_venue(tgb_s* t, tgbSendVenue_s* opz);
```

telegram method sendContact
```
err_t tgb_send_contact(tgb_s* t, tgbSendContact_s* opz);
```

telegram method getFile
```
err_t tgb_get_file(tgb_s* t, tgbGetFile_s* opz);
```

telegram method kick
```
err_t tgb_ban(tgb_s* t, tgbBan_s* opz);
```

telegram method leave
```
err_t tgb_leave(tgb_s* t, tgbLeave_s* opz);
```

telegram method unban
```
err_t tgb_unban(tgb_s* t, tgbUnban_s* opz);
```

telegram method getChat
```
err_t tgb_get_chat(tgbChat_s* out, tgb_s* t, tgbGetChat_s* opz);
```

telegram method getChatAdministrator
```
tgbChatMember_s* tgb_get_chat_administrator(uint_t* countOut, tgb_s* t, tgbGetChat_s* opz);
```

telegram method getChatMember
```
err_t tgb_get_chat_member(tgbChatMember_s* out, tgb_s* t, tgbGetChatMember_s* opz);
```

init multithreading, with max "count" thread
```
err_t tgb_async_init(tgbAsync_s* ta, uint_t count);
```

send action to thread, thread autoresume and execute tgbAction_f function
```
err_t tgb_async_action(tgb_s* t, tgbAsync_s* ta, tgbAction_f fnc, tgbUpdate_s* up);
```

call _end after end of call all actions.
```
err_t tgb_async_end(tgb_s* t, tgbAsync_s* ta);
```

release multithread resources.
```
err_t tgb_async_destroy(tgbAsync_s* ta);
```


### Structure

you can view structure in api.h file<br/>
the elements names of the structures reflect those of the messages received by [telegram](https://core.telegram.org/bots/api)<br/>
check element NULL if mark optional.

## Version

v0.0

## Authors

* **vbextreme**

## License
* copyright **vbextreme 2017**

This project is licensed under the GPL v3 License - see the [LICENSE.md](LICENSE.md) file for details





