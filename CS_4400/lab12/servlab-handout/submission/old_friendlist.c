/*
 * Harrison Quick - u1098604
 * 
 * friendlist.c - [Starting code for] a web-based friend-graph manager.
 *
 * Based on:
 *  tiny.c - A simple, iterative HTTP/1.0 Web server that uses the 
 *      GET method to serve static and dynamic content.
 *   Tiny Web server
 *   Dave O'Hallaron
 *   Carnegie Mellon University
 */
#include "csapp.h"
#include "dictionary.h"
#include "more_string.h"

static void doit(int fd);
static void *Doit(void *fd);
static dictionary_t *read_requesthdrs(rio_t *rp);
static void read_postquery(rio_t *rp, dictionary_t *headers, dictionary_t *d);
static void clienterror(int fd, char *cause, char *errnum, 
                        char *shortmsg, char *longmsg);
static void serve_request(int fd, dictionary_t *query);
static void serve_greet(int fd, dictionary_t *query);
static void serve_friends(int fd, dictionary_t *query);
static void serve_befriend(int fd, dictionary_t *query, int remover);
static void sig_int_handler(int signum);
static void serve_introduce(int fd, dictionary_t *query);

static dictionary_t *server_dict;
static sem_t ready_sem, dict_sem;

static void sig_int_handler(int signum)
{
  free_dictionary(server_dict);
  exit(0);
}

int main(int argc, char **argv) {
  int listenfd, connfd, *connfd_p;
  socklen_t clientlen;
  struct sockaddr_storage clientaddr;
  pthread_t th;

  server_dict = make_dictionary(COMPARE_CASE_SENS, (free_proc_t)free_dictionary);
  Sem_init(&dict_sem, 0, 1); // For creating signals
  Sem_init(&ready_sem, 0, 0);  // For accessing global dictionary

  /* Check command line args */
  if (argc != 2) {
    fprintf(stderr, "usage: %s <port>\n", argv[0]);
    exit(1);
  }

  listenfd = Open_listenfd(argv[1]);

  /* Don't kill the server if there's an error, because
     we want to survive errors due to a client. But we
     do want to report errors. */
  exit_on_error(0);

  /* Also, don't stop on broken connections: */
  Signal(SIGPIPE, SIG_IGN);
  Signal(SIGINT, sig_int_handler);

  while (1) {
    clientlen = sizeof(clientaddr);
    connfd = Accept(listenfd, (SA *)&clientaddr, &clientlen);
    if (connfd >= 0) {
      connfd_p = malloc(sizeof(int));
      *connfd_p = connfd;
      Pthread_create(&th, NULL, Doit, connfd_p);
      P(&ready_sem);
      Pthread_detach(th);
    }
  }
}

void *Doit(void *fd_v) {
  int fd = *(int *)fd_v;
  free(fd_v);
  doit(fd);
  Close(fd);
  return NULL;
}

/*
 * doit - handle one HTTP request/response transaction
 */
void doit(int fd) {
  
 
  char buf[MAXLINE], *method, *uri, *version, *uri_decoded;
  rio_t rio;
  dictionary_t *headers, *query;

  /* Read request line and headers */
  Rio_readinitb(&rio, fd);
  V(&ready_sem);
  if (Rio_readlineb(&rio, buf, MAXLINE) <= 0)
    return;

  if (!parse_request_line(buf, &method, &uri, &version)) {
    clienterror(fd, method, "400", "Bad Request",
                "Friendlist did not recognize the request");
  } else {
    if (strcasecmp(version, "HTTP/1.0")
        && strcasecmp(version, "HTTP/1.1")) {
      clienterror(fd, version, "501", "Not Implemented",
                  "Friendlist does not implement that version");
    } else if (strcasecmp(method, "GET")
               && strcasecmp(method, "POST")) {
      clienterror(fd, method, "501", "Not Implemented",
                  "Friendlist does not implement that method");
    } else {
      headers = read_requesthdrs(&rio);

      /* Parse all query arguments into a dictionary */
      query = make_dictionary(COMPARE_CASE_SENS, free);
      uri_decoded = query_decode(uri);
      parse_uriquery(uri_decoded, query);
      free(uri_decoded);
      if (!strcasecmp(method, "POST"))
        read_postquery(&rio, headers, query);
      
      /* You'll want to handle different queries here,
         but the intial implementation always returns
         nothing: */
      if (starts_with("/greet", uri))
	serve_greet(fd, query);
      else if (starts_with("/friends", uri))
	serve_friends(fd, query);
      else if (starts_with("/befriend", uri))
	serve_befriend(fd, query, 0);
      else if (starts_with("/unfriend", uri))
	serve_befriend(fd, query, 1);
      else if (starts_with("/introduce", uri))
	serve_introduce(fd, query);
      else 
	serve_request(fd, query);

      /* Clean up */
      free_dictionary(query);
      free_dictionary(headers);
    }

    /* Clean up status line */
    free(method);
    free(uri);
    free(version);
  }
  return;
}

/*
 * read_requesthdrs - read HTTP request headers
 */
dictionary_t *read_requesthdrs(rio_t *rp) {
  char buf[MAXLINE];
  dictionary_t *d = make_dictionary(COMPARE_CASE_INSENS, free);

  Rio_readlineb(rp, buf, MAXLINE);
  
  while(strcmp(buf, "\r\n")) {
    Rio_readlineb(rp, buf, MAXLINE);
    parse_header_line(buf, d);
  }
  
  return d;
}

void read_postquery(rio_t *rp, dictionary_t *headers, dictionary_t *dest) {
  char *len_str, *type, *buffer;
  int len;
  
  len_str = dictionary_get(headers, "Content-Length");
  len = (len_str ? atoi(len_str) : 0);

  type = dictionary_get(headers, "Content-Type");
  
  buffer = malloc(len+1);
  Rio_readnb(rp, buffer, len);
  buffer[len] = 0;

  if (!strcasecmp(type, "application/x-www-form-urlencoded"))
    parse_query(buffer, dest);

  free(buffer);
}

static char *ok_header(size_t len, const char *content_type) {
  char *len_str, *header;
  
  header = append_strings("HTTP/1.0 200 OK\r\n",
                          "Server: Friendlist Web Server\r\n",
                          "Connection: close\r\n",
                          "Content-length: ", len_str = to_string(len), "\r\n",
                          "Content-type: ", content_type, "\r\n\r\n",
                          NULL);
  free(len_str);

  return header;
}

static char *request_header(size_t len, const char *path, const char *host) {
  char *header;
  
  header = append_strings("GET ", path, " HTTP/1.0\r\n",  
			  "Host: ", host, "\r\n",
			  "Connection: close\r\n",
			  "Accept: text/html\r\n",
			  "Accept-Encoding: gzip, deflate, br\r\n",
			  "Accept-Language: en-US,en;q=0.9\r\n\r\n",
			  NULL);
  
  return header;
}

/*
 * serve_request - example request handler
 */
static void serve_request(int fd, dictionary_t *query) {
  size_t len;
  char *body, *header;

  body = strdup("alice\nbob");

  len = strlen(body);

  /* Send response headers to client */
  header = ok_header(len, "text/html; charset=utf-8");
  Rio_writen(fd, header, strlen(header));
  free(header);
  
  /* Send response body to client */
  Rio_writen(fd, body, len);
  free(body);
}

/*Serve Greet Handler*/
static void serve_greet(int fd, dictionary_t *query) {
  size_t len;
  char *body, *header, *user;

  user = dictionary_get(query, "user");
  body = append_strings("Greetings, ", user, "!", NULL);

  len = strlen(body);

  /* Send response headers to client */
  header = ok_header(len, "text/html; charset=utf-8");
  Rio_writen(fd, header, strlen(header));
  free(header);

  /* Send response body to client */
  Rio_writen(fd, body, len);
  free(body);
}

/*Serve Friends Handler*/
static void serve_friends(int fd, dictionary_t *query) {
  size_t len;
  char *body, *realBody, *header;
  char *user = NULL;
  dictionary_t *friends = NULL;
  int malloced = 0;

  user = dictionary_get(query, "user");

  if (user)
    {
      P(&dict_sem);
      friends = (dictionary_t *)dictionary_get(server_dict, user);
      realBody = "";
      body = "";
      int i = 0;
      if (friends)
	{
	  size_t count = dictionary_count(friends);
      
	  for (; i < count; i++)
	    {
	      body = append_strings(realBody, dictionary_key(friends, i), "\n", NULL);
	      if (malloced)
		free(realBody);
	      realBody = (char *)malloc(strlen(body) + 1);
	      malloced = 1;
	      strcpy(realBody, body);
	      free(body);
	    }
	}
      V(&dict_sem);
    }
  else
    return;
  
  len = strlen(realBody);

  //Send response headers to client
  header = ok_header(len, "text/html; charset=utf-8");
  Rio_writen(fd, header, strlen(header));

  free(header);

  //Send response body to client 
  Rio_writen(fd, realBody, len);

  if (malloced)
    free(realBody);
}

/*
 * serve_befriend handler
 */
static void serve_befriend(int fd, dictionary_t *query, int remove) {
  size_t len;
  char *body, *realBody, *header, *user, *friend;
  char **friendsToAdd;
  dictionary_t *friends;

  user = dictionary_get(query, "user");
  friend = query_decode(dictionary_get(query, "friends"));
  friendsToAdd = split_string(friend, '\n');
  free(friend);
  char **friendsToAdd2 = friendsToAdd;

  if (!user || !friend || !friendsToAdd) // Bad request.
    return;

  P(&dict_sem);
  // Begin by iterating over list of friends to be added.
  for (char *friendToAdd = *friendsToAdd; friendToAdd; friendToAdd = *++friendsToAdd)
    {
      if (!strcmp(user, friendToAdd))
	{
	  free(friendToAdd);
	  continue;
	}
      
      
      if (!dictionary_get(server_dict, friendToAdd)) // Server doesn't have entry for friend
	{
	  if (!remove)
	    {
	      // Add entry for friend
	      dictionary_t *newSet = make_dictionary(COMPARE_CASE_SENS, NULL);
	      // Add user to friend's friendlist
	      dictionary_set(newSet, user, NULL);
	      // Put new user's entry in server's dictionary
	      dictionary_set(server_dict, friendToAdd, newSet);
	    }
	}
      else // Server does have entry for friend!
	{
	  dictionary_t *newFriendsFriends = (dictionary_t *)dictionary_get(
						  server_dict, friendToAdd);
	  
	  if (!dictionary_get(newFriendsFriends, user) && !remove) // Only add user if not redundant
	    dictionary_set(newFriendsFriends, user, NULL);
	  else if (remove) // Remove if it's in there!
	    {
	      dictionary_remove(newFriendsFriends, user);
	      
	      if (dictionary_count(newFriendsFriends) < 1)
		{
		  dictionary_remove(server_dict, friendToAdd);
		}
	    }
	}

      if (!dictionary_get(server_dict, user)) // Server doesn't have entry for user
	{
	  if (!remove)
	    {
	      // Add entry for user
	      dictionary_t *newSet = make_dictionary(COMPARE_CASE_SENS, NULL);
	      // Add new friend to user's friendlist
	      dictionary_set(newSet, friendToAdd, NULL);
	      // Put new user's entry in server's dictionary
	      dictionary_set(server_dict, user, newSet);
	    }
	}
      else // Server does have entry for user!
	{
	  dictionary_t *currFriends = (dictionary_t *)dictionary_get(server_dict, user);
	  
	  if (!dictionary_get(currFriends, friendToAdd) && !remove) // Only add friend if not redundant
	    dictionary_set(currFriends, friendToAdd, NULL);
	  else if (remove)
	    {
	      dictionary_remove(currFriends, friendToAdd);
	      if (dictionary_count(currFriends) < 1)
		  dictionary_remove(server_dict, user);
	    }
	}
      free(friendToAdd);
    }
  V(&dict_sem);
  free(friendsToAdd2);

  realBody = "";
  body = "";
  int i = 0;
  int malloced = 0;

  P(&dict_sem);
  friends = (dictionary_t *)dictionary_get(server_dict, user);
  
  if (friends)
    {
      size_t count = dictionary_count(friends);
      
      for (; i < count; i++)
	{
	  if (realBody)
	      body = append_strings(realBody, dictionary_key(friends, i), "\n", NULL);
	  if (malloced)
	    {
	      free(realBody);
	      malloced = 0;
	    }
	  if (strcmp(body, "") && body)
	    {
	      realBody = (char *)malloc(strlen(body) + 1);
	      malloced = 1;
	      strcpy(realBody, body);
	      free(body);
	    }
	}
    }
  V(&dict_sem);
  
  len = strlen(realBody);

  //Send response headers to client
  header = ok_header(len, "text/html; charset=utf-8");
  Rio_writen(fd, header, strlen(header));

  free(header);

  //Send response body to client 
  Rio_writen(fd, realBody, len);

  if (malloced)
    free(realBody);
}
/*
 * serve_introduce - introduce handler
 */
static void serve_introduce(int fd, dictionary_t *query) {
  size_t len;
  char *body, *header, *user, *friend, *hostname, *port;
  char **name, **name_p, **status_p;
  int socket_fd = 0;
  rio_t rio;
  char buf[MAXLINE];
  dictionary_t *userFriends, *friendsFriends;

  user = dictionary_get(query, "user");        // Initialize everything
  friend = dictionary_get(query, "friend");
  hostname = dictionary_get(query, "host");
  port = dictionary_get(query, "port");
  socket_fd = Open_clientfd(hostname, port);
  
  if (!user || !friend || !hostname || !port || !socket_fd) // Bad request
    return;

  //////////////////////////////////////////////////
  // Asking for friends of User
  //////////////////////////////////////////////////
  
  body = append_strings("/friends?user=", user, NULL);
  len = strlen(user);

  // Send header to other server
  header = request_header(len, body, hostname);
  Rio_writen(socket_fd, header, strlen(header));
  free(body);
  free(header);
  
  Rio_readinitb(&rio, socket_fd);
  if (Rio_readlineb(&rio, buf, MAXLINE) <= 0) //Responds OK <-----
    return;

  status_p = malloc(sizeof(char **));
  if (parse_status_line(buf, NULL, status_p, NULL)) // Check to make sure OK
    {
      if (strcmp(*status_p, "200"))
	{
	  free(*status_p);
	  free(status_p);
	  return;
	}
      free(*status_p);
    }
  else
      return;

  free(status_p);
  
  if (Rio_readlineb(&rio, buf, MAXLINE) <= 0)
    return;
  if (Rio_readlineb(&rio, buf, MAXLINE) <= 0)
    return;
  if (Rio_readlineb(&rio, buf, MAXLINE) <= 0) // length <------
    return;

  dictionary_t *d = make_dictionary(COMPARE_CASE_SENS, free);
  parse_header_line(buf, d);
  char *content_len = dictionary_get(d, "Content-length");
  int  content_len_int = atoi(content_len);
  //free(content_len);

  if (content_len_int != 0) // No new friends to report if == '0'
    {
      free_dictionary(d);

      if (Rio_readlineb(&rio, buf, MAXLINE) <= 0) // Content - type
	return;
      if (Rio_readlineb(&rio, buf, MAXLINE) <= 0) // \r\n
	return;

      P(&dict_sem);
      if (!dictionary_get(server_dict, friend)) // Same for friend
	{
	  friendsFriends = make_dictionary(COMPARE_CASE_SENS, NULL);
	  dictionary_set(friendsFriends, user, NULL);
	  dictionary_set(server_dict, friend, friendsFriends);
	}
      else
	friendsFriends = dictionary_get(server_dict, friend);
      
      while(content_len_int)
	{
	  if (Rio_readlineb(&rio, buf, MAXLINE) <= 0) // fetch name
	    return;

	  name = split_string(buf, '\n');
	  name_p = name;

	  content_len_int -= (strlen(*name) + 1); // subtract len of relevant buffer

	  if (!dictionary_get(friendsFriends, *name) && strcmp(*name, friend)) // New friend of user?
	      dictionary_set(friendsFriends, *name, NULL); // Add entry if so
	  //if (!dictionary_get(friendsFriends, *name)) // New friend of user?
	      //dictionary_set(friendsFriends, *name, NULL); // Add entry if so

	  for (char *stringToFree = *name; stringToFree; stringToFree = *++name)
	    free(*name); // Just freeing up name
	  free(name_p);
	}
      V(&dict_sem);
    }
  else 
    free_dictionary(d); // Still need to free d
  
  //////////////////////////////////////////////////
  // Asking for friends of Friend
  //////////////////////////////////////////////////

  Close(socket_fd);
  socket_fd = Open_clientfd(hostname, port);

  body = append_strings("/friends?user=", friend, NULL);
  len = strlen(friend);

  // Send header to other server
  header = request_header(len, body, hostname);
  Rio_writen(socket_fd, header, strlen(header));
  free(body);
  free(header);
  
  Rio_readinitb(&rio, socket_fd);
  if (Rio_readlineb(&rio, buf, MAXLINE) <= 0)
    return;
  if (Rio_readlineb(&rio, buf, MAXLINE) <= 0)
    return;
  if (Rio_readlineb(&rio, buf, MAXLINE) <= 0)
    return;
  if (Rio_readlineb(&rio, buf, MAXLINE) <= 0) // length <------
    return;

  d = make_dictionary(COMPARE_CASE_SENS, free);
  parse_header_line(buf, d);
  content_len = dictionary_get(d, "Content-length");
  content_len_int = atoi(content_len);

  if (content_len_int != 0) // No new friends to report if == '0'
    {
      free_dictionary(d);

      if (Rio_readlineb(&rio, buf, MAXLINE) <= 0) // Content - type
	return;
      if (Rio_readlineb(&rio, buf, MAXLINE) <= 0) // \r\n
	return;

      P(&dict_sem);
      if (!dictionary_get(server_dict, user)) // Make sure we have entry for user
	{
	  userFriends = make_dictionary(COMPARE_CASE_SENS, NULL);
	  dictionary_set(userFriends, friend, NULL);
	  dictionary_set(server_dict, user, userFriends);
	}
      else
	userFriends = dictionary_get(server_dict, user);
      
      while(content_len_int)
	{
	  if (Rio_readlineb(&rio, buf, MAXLINE) <= 0) // fetch name
	    return;

	  name = split_string(buf, '\n');
	  name_p = name;

	  content_len_int -= (strlen(*name) + 1); // subtract len of relevant buffer

	  if (!dictionary_get(userFriends, *name) && strcmp(*name, user)) // New friend of user?
	      dictionary_set(userFriends, *name, NULL); // Add entry if so

	  for (char *stringToFree = *name; stringToFree; stringToFree = *++name)
	    free(*name); // Just freeing up name
	  free(name_p);
	}
      V(&dict_sem);
    }
  else 
    free_dictionary(d); // Still need to free d

  Close(socket_fd);

  body = "Nice to meet you!\n";

  //Send response headers to client
  header = ok_header(strlen(body), "text/html; charset=utf-8");
  Rio_writen(fd, header, strlen(header));

  free(header);
  Rio_writen(fd, body, len);
}


/*
 * clienterror - returns an error message to the client
 */
void clienterror(int fd, char *cause, char *errnum, 
		 char *shortmsg, char *longmsg) {
  size_t len;
  char *header, *body, *len_str;

  body = append_strings("<html><title>Friendlist Error</title>",
                        "<body bgcolor=""ffffff"">\r\n",
                        errnum, " ", shortmsg,
                        "<p>", longmsg, ": ", cause,
                        "<hr><em>Friendlist Server</em>\r\n",
                        NULL);
  len = strlen(body);

  /* Print the HTTP response */
  header = append_strings("HTTP/1.0 ", errnum, " ", shortmsg, "\r\n",
                          "Content-type: text/html; charset=utf-8\r\n",
                          "Content-length: ", len_str = to_string(len), "\r\n\r\n",
                          NULL);
  free(len_str);
  
  Rio_writen(fd, header, strlen(header));
  Rio_writen(fd, body, len);

  free(header);
  free(body);
}
