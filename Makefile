CFLAGS += -Werror -Wall
all: check_user pam_http.so
clean:
	$(RM) check_user pam_http.so *.o
pam_http.so: pam_http.c
	$(CC) $(CFLAGS) -fPIC -shared -Xlinker -x -o $@ $< -lcurl
check_user: check_user.c
	$(CC) $(CFLAGS) -o $@ $< -lpam -lpam_misc