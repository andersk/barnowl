#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <time.h>
#include "owl.h"

static const char fileIdent[] = "$Id$";

void owl_message_init(owl_message *m)
{
  time_t t;

  m->id=owl_global_get_nextmsgid(&g);
  m->type=OWL_MESSAGE_TYPE_GENERIC;
  owl_message_set_direction_none(m);
  m->delete=0;
  strcpy(m->hostname, "");
  m->zwriteline=strdup("");
  m->invalid_format=1;

  owl_list_create(&(m->attributes));
  
  /* save the time */
  t=time(NULL);
  m->time=owl_strdup(ctime(&t));
  m->time[strlen(m->time)-1]='\0';
  owl_fmtext_init_null(&(m->fmtext));
}

void owl_message_set_attribute(owl_message *m, char *attrname, char *attrvalue)
{
  /* add the named attribute to the message.  If an attribute with the
     name already exists, replace the old value with the new value */

  int i, j;
  owl_pair *p;

  /* look for an existing pair with this key, and nuke the entry if
     found */
  j=owl_list_get_size(&(m->attributes));
  for (i=0; i<j; i++) {
    p=owl_list_get_element(&(m->attributes), i);
    if (!strcmp(owl_pair_get_key(p), attrname)) {
      owl_free(owl_pair_get_key(p));
      owl_free(owl_pair_get_value(p));
      owl_free(p);
      owl_list_remove_element(&(m->attributes), i);
      break;
    }
  }

  p=owl_malloc(sizeof(owl_pair));
  owl_pair_create(p, owl_strdup(attrname), owl_strdup(attrvalue));
  owl_list_append_element(&(m->attributes), p);
}

char *owl_message_get_attribute_value(owl_message *m, char *attrname)
{
  /* return the value associated with the named attribute, or NULL if
     the attribute does not exist */
  int i, j;
  owl_pair *p;

  j=owl_list_get_size(&(m->attributes));
  for (i=0; i<j; i++) {
    p=owl_list_get_element(&(m->attributes), i);
    if (!strcmp(owl_pair_get_key(p), attrname)) {
      return(owl_pair_get_value(p));
    }
  }
  owl_function_debugmsg("No attribute %s found", attrname);
  return(NULL);
}

/* We cheat and indent it for now, since we really want this for
 * the 'info' function.  Later there should just be a generic
 * function to indent fmtext.
 */
void owl_message_attributes_tofmtext(owl_message *m, owl_fmtext *fm) {
  int i, j;
  owl_pair *p;
  char *buff;

  owl_fmtext_init_null(fm);

  j=owl_list_get_size(&(m->attributes));
  for (i=0; i<j; i++) {
    p=owl_list_get_element(&(m->attributes), i);
    buff=owl_sprintf("  %-15.15s: %-35.35s\n", owl_pair_get_key(p), owl_pair_get_value(p));
    owl_fmtext_append_normal(fm, buff);
    owl_free(buff);
  }
}

void owl_message_invalidate_format(owl_message *m)
{
  m->invalid_format=1;
}

owl_fmtext *owl_message_get_fmtext(owl_message *m)
{
  owl_message_format(m);
  return(&(m->fmtext));
}

void owl_message_format(owl_message *m)
{
  owl_style *s;
  owl_view *v;

  if (m->invalid_format) {
    /* for now we assume there's jsut the one view and use that style */
    v=owl_global_get_current_view(&g);
    s=owl_view_get_style(v);

    owl_fmtext_free(&(m->fmtext));
    owl_fmtext_init_null(&(m->fmtext));
    owl_style_get_formattext(s, &(m->fmtext), m);
    m->invalid_format=0;
  }
}

void owl_message_set_class(owl_message *m, char *class)
{
  owl_message_set_attribute(m, "class", class);
}

char *owl_message_get_class(owl_message *m)
{
  char *class;

  class=owl_message_get_attribute_value(m, "class");
  if (!class) return("");
  return(class);
}

void owl_message_set_instance(owl_message *m, char *inst)
{
  owl_message_set_attribute(m, "instance", inst);
}

char *owl_message_get_instance(owl_message *m)
{
  char *instance;

  instance=owl_message_get_attribute_value(m, "instance");
  if (!instance) return("");
  return(instance);
}

void owl_message_set_sender(owl_message *m, char *sender)
{
  owl_message_set_attribute(m, "sender", sender);
}

char *owl_message_get_sender(owl_message *m)
{
  char *sender;

  sender=owl_message_get_attribute_value(m, "sender");
  if (!sender) return("");
  return(sender);
}

void owl_message_set_zsig(owl_message *m, char *zsig)
{
  owl_message_set_attribute(m, "zsig", zsig);
}

char *owl_message_get_zsig(owl_message *m)
{
  char *zsig;

  zsig=owl_message_get_attribute_value(m, "zsig");
  if (!zsig) return("");
  return(zsig);
}

void owl_message_set_recipient(owl_message *m, char *recip)
{
  owl_message_set_attribute(m, "recipient", recip);
}

char *owl_message_get_recipient(owl_message *m)
{
  /* this is stupid for outgoing messages, we need to fix it. */

  char *recip;

  recip=owl_message_get_attribute_value(m, "recipient");
  if (!recip) return("");
  return(recip);
}

void owl_message_set_realm(owl_message *m, char *realm)
{
  owl_message_set_attribute(m, "realm", realm);
}

char *owl_message_get_realm(owl_message *m)
{
  char *realm;
  
  realm=owl_message_get_attribute_value(m, "realm");
  if (!realm) return("");
  return(realm);
}

void owl_message_set_body(owl_message *m, char *body)
{
  owl_message_set_attribute(m, "body", body);
}

char *owl_message_get_body(owl_message *m)
{
  char *body;

  body=owl_message_get_attribute_value(m, "body");
  if (!body) return("");
  return(body);
}


void owl_message_set_opcode(owl_message *m, char *opcode)
{
  owl_message_set_attribute(m, "opcode", opcode);
}

char *owl_message_get_opcode(owl_message *m)
{
  char *opcode;

  opcode=owl_message_get_attribute_value(m, "opcode");
  if (!opcode) return("");
  return(opcode);
}


void owl_message_set_islogin(owl_message *m)
{
  owl_message_set_attribute(m, "loginout", "login");
}


void owl_message_set_islogout(owl_message *m)
{
  owl_message_set_attribute(m, "loginout", "logout");
}

int owl_message_is_loginout(owl_message *m)
{
  char *res;

  res=owl_message_get_attribute_value(m, "loginout");
  if (!res) return(0);
  return(1);
}

int owl_message_is_login(owl_message *m)
{
  char *res;

  res=owl_message_get_attribute_value(m, "loginout");
  if (!res) return(0);
  if (!strcmp(res, "login")) return(1);
  return(0);
}


int owl_message_is_logout(owl_message *m)
{
  char *res;

  res=owl_message_get_attribute_value(m, "loginout");
  if (!res) return(0);
  if (!strcmp(res, "logout")) return(1);
  return(0);
}

void owl_message_set_isprivate(owl_message *m)
{
  owl_message_set_attribute(m, "isprivate", "");
}

int owl_message_is_private(owl_message *m)
{
  char *res;

  res=owl_message_get_attribute_value(m, "isprivate");
  if (!res) return(0);
  return(1);
}

char *owl_message_get_timestr(owl_message *m)
{
  return(m->time);
}

void owl_message_set_type_admin(owl_message *m)
{
  m->type=OWL_MESSAGE_TYPE_ADMIN;
}

void owl_message_set_type_zephyr(owl_message *m)
{
  m->type=OWL_MESSAGE_TYPE_ZEPHYR;
}

void owl_message_set_type_aim(owl_message *m)
{
  m->type=OWL_MESSAGE_TYPE_AIM;
}
						
int owl_message_is_type_admin(owl_message *m)
{
  if (m->type==OWL_MESSAGE_TYPE_ADMIN) return(1);
  return(0);
}

int owl_message_is_type_zephyr(owl_message *m)
{
  if (m->type==OWL_MESSAGE_TYPE_ZEPHYR) return(1);
  return(0);
}

int owl_message_is_type_aim(owl_message *m)
{
  if (m->type==OWL_MESSAGE_TYPE_AIM) return(1);
  return(0);
}

int owl_message_is_type_generic(owl_message *m)
{
  if (m->type==OWL_MESSAGE_TYPE_GENERIC) return(1);
  return(0);
}

char *owl_message_type_to_string(owl_message *m)
{
  if (m->type==OWL_MESSAGE_TYPE_ADMIN) return("admin");
  if (m->type==OWL_MESSAGE_TYPE_GENERIC) return("generic");
  if (m->type==OWL_MESSAGE_TYPE_ZEPHYR) return("zephyr");
  if (m->type==OWL_MESSAGE_TYPE_AIM) return("aim");
  if (m->type==OWL_MESSAGE_TYPE_JABBER) return("jabber");
  if (m->type==OWL_MESSAGE_TYPE_ICQ) return("icq");
  if (m->type==OWL_MESSAGE_TYPE_MSN) return("msn");
  return("unknown");
}

char *owl_message_get_text(owl_message *m)
{
  return(owl_fmtext_get_text(&(m->fmtext)));
}

void owl_message_set_direction_in(owl_message *m)
{
  m->direction=OWL_MESSAGE_DIRECTION_IN;
}

void owl_message_set_direction_out(owl_message *m)
{
  m->direction=OWL_MESSAGE_DIRECTION_OUT;
}

void owl_message_set_direction_none(owl_message *m)
{
  m->direction=OWL_MESSAGE_DIRECTION_NONE;
}

int owl_message_is_direction_in(owl_message *m)
{
  if (m->direction==OWL_MESSAGE_DIRECTION_IN) return(1);
  return(0);
}

int owl_message_is_direction_out(owl_message *m)
{
  if (m->direction==OWL_MESSAGE_DIRECTION_OUT) return(1);
  return(0);
}

int owl_message_is_direction_none(owl_message *m)
{
  if (m->direction==OWL_MESSAGE_DIRECTION_NONE) return(1);
  return(0);
}

int owl_message_get_numlines(owl_message *m)
{
  if (m == NULL) return(0);
  owl_message_format(m);
  return(owl_fmtext_num_lines(&(m->fmtext)));
}

void owl_message_mark_delete(owl_message *m)
{
  if (m == NULL) return;
  m->delete=1;
}

void owl_message_unmark_delete(owl_message *m)
{
  if (m == NULL) return;
  m->delete=0;
}

char *owl_message_get_zwriteline(owl_message *m)
{
  return(m->zwriteline);
}

void owl_message_set_zwriteline(owl_message *m, char *line)
{
  m->zwriteline=strdup(line);
}

int owl_message_is_delete(owl_message *m)
{
  if (m == NULL) return(0);
  if (m->delete==1) return(1);
  return(0);
}

#ifdef HAVE_LIBZEPHYR
ZNotice_t *owl_message_get_notice(owl_message *m)
{
  return(&(m->notice));
}
#else
void *owl_message_get_notice(owl_message *m)
{
  return(NULL);
}
#endif

char *owl_message_get_hostname(owl_message *m)
{
  return(m->hostname);
}


void owl_message_curs_waddstr(owl_message *m, WINDOW *win, int aline, int bline, int acol, int bcol, int color)
{
  owl_fmtext a, b;

  /* this will ensure that our cached copy is up to date */
  owl_message_format(m);

  owl_fmtext_init_null(&a);
  owl_fmtext_init_null(&b);
  
  owl_fmtext_truncate_lines(&(m->fmtext), aline, bline-aline+1, &a);
  owl_fmtext_truncate_cols(&a, acol, bcol, &b);
  if (color!=OWL_COLOR_DEFAULT) {
    owl_fmtext_colorize(&b, color);
  }

  if (owl_global_is_search_active(&g)) {
    owl_fmtext_search_and_highlight(&b, owl_global_get_search_string(&g));
  }
      
  owl_fmtext_curs_waddstr(&b, win);

  owl_fmtext_free(&a);
  owl_fmtext_free(&b);
}

int owl_message_is_personal(owl_message *m)
{
  if (owl_message_is_type_zephyr(m)) {
    if (strcasecmp(owl_message_get_class(m), "message")) return(0);
    if (strcasecmp(owl_message_get_instance(m), "personal")) return(0);
    if (!strcasecmp(owl_message_get_recipient(m), owl_zephyr_get_sender()) ||
	!strcasecmp(owl_message_get_sender(m), owl_zephyr_get_sender())) {
      return(1);
    }
  }
  return(0);
}

int owl_message_is_from_me(owl_message *m)
{
  if (owl_message_is_type_zephyr(m)) {
    if (!strcasecmp(owl_message_get_sender(m), owl_zephyr_get_sender())) {
      return(1);
    } else {
      return(0);
    }
  } else if (owl_message_is_type_aim(m)) {
    if (!strcasecmp(owl_message_get_sender(m), owl_global_get_aim_screenname(&g))) {
      return(1);
    } else {
      return(0);
    }
  } else if (owl_message_is_type_admin(m)) {
    return(0);
  }
  return(0);
}

int owl_message_is_mail(owl_message *m)
{
  if (owl_message_is_type_zephyr(m)) {
    if (!strcasecmp(owl_message_get_class(m), "mail") && owl_message_is_private(m)) {
      return(1);
    } else {
      return(0);
    }
  }
  return(0);
}

int owl_message_is_ping(owl_message *m)
{
  if (owl_message_is_type_zephyr(m)) {
    if (!strcasecmp(owl_message_get_opcode(m), "ping")) {
      return(1);
    } else {
      return(0);
    }
  }
  return(0);
}

int owl_message_is_burningears(owl_message *m)
{
  /* we should add a global to cache the short zsender */
  char sender[LINE], *ptr;

  /* if the message is from us or to us, it doesn't count */
  if (owl_message_is_from_me(m) || owl_message_is_private(m)) return(0);

  if (owl_message_is_type_zephyr(m)) {
    strcpy(sender, owl_zephyr_get_sender());
    ptr=strchr(sender, '@');
    if (ptr) *ptr='\0';
  } else if (owl_message_is_type_aim(m)) {
    strcpy(sender, owl_global_get_aim_screenname(&g));
  } else {
    return(0);
  }

  if (stristr(owl_message_get_body(m), sender)) {
    return(1);
  }
  return(0);
}

/* caller must free return value. */
char *owl_message_get_cc(owl_message *m)
{
  char *cur, *out, *end;

  cur = owl_message_get_body(m);
  while (*cur && *cur==' ') cur++;
  if (strncasecmp(cur, "cc:", 3)) return(NULL);
  cur+=3;
  while (*cur && *cur==' ') cur++;
  out = owl_strdup(cur);
  end = strchr(out, '\n');
  if (end) end[0] = '\0';
  return(out);
}

int owl_message_get_id(owl_message *m)
{
  return(m->id);
}

char *owl_message_get_type(owl_message *m) {
  switch (m->type) {
  case OWL_MESSAGE_TYPE_ADMIN:
    return("admin");
  case OWL_MESSAGE_TYPE_ZEPHYR:
    return("zephyr");
  case OWL_MESSAGE_TYPE_GENERIC:
    return("generic");
  case OWL_MESSAGE_TYPE_AIM:
    return("aim");
  case OWL_MESSAGE_TYPE_JABBER:
    return("jabber");
  case OWL_MESSAGE_TYPE_ICQ:
    return("icq");
  case OWL_MESSAGE_TYPE_YAHOO:
    return("yahoo");
  case OWL_MESSAGE_TYPE_MSN:
    return("msn");
  default:
    return("unknown");
  }
}

char *owl_message_get_direction(owl_message *m) {
  switch (m->direction) {
  case OWL_MESSAGE_DIRECTION_IN:
    return("in");
  case OWL_MESSAGE_DIRECTION_OUT:
    return("out");
  case OWL_MESSAGE_DIRECTION_NONE:
    return("none");
  default:
    return("unknown");
  }
}

char *owl_message_get_login(owl_message *m) {
  if (owl_message_is_login(m)) {
    return "login";
  } else if (owl_message_is_logout(m)) {
    return "logout";
  } else {
    return "none";
  }
}

char *owl_message_get_header(owl_message *m) {
  return owl_message_get_attribute_value(m, "adminheader");
}

/* return 1 if the message contains "string", 0 otherwise.  This is
 * case insensitive because the functions it uses are
 */
int owl_message_search(owl_message *m, char *string)
{

  owl_message_format(m); /* is this necessary? */
  
  return (owl_fmtext_search(&(m->fmtext), string));
}


/* if loginout == -1 it's a logout message
 *                 0 it's not a login/logout message
 *                 1 it's a login message
 */
void owl_message_create_aim(owl_message *m, char *sender, char *recipient, char *text, int direction, int loginout)
{
  owl_message_init(m);
  owl_message_set_body(m, text);
  owl_message_set_sender(m, sender);
  owl_message_set_recipient(m, recipient);
  owl_message_set_type_aim(m);

  if (direction==OWL_MESSAGE_DIRECTION_IN) {
    owl_message_set_direction_in(m);
  } else if (direction==OWL_MESSAGE_DIRECTION_OUT) {
    owl_message_set_direction_out(m);
  }

  /* for now all messages that aren't loginout are private */
  if (!loginout) {
    owl_message_set_isprivate(m);
  }

  if (loginout==-1) {
    owl_message_set_islogout(m);
  } else if (loginout==1) {
    owl_message_set_islogin(m);
  }
}

void owl_message_create_admin(owl_message *m, char *header, char *text)
{
  owl_message_init(m);
  owl_message_set_type_admin(m);
  owl_message_set_body(m, text);
  owl_message_set_attribute(m, "adminheader", header); /* just a hack for now */
}

#ifdef HAVE_LIBZEPHYR
void owl_message_create_from_znotice(owl_message *m, ZNotice_t *n)
{
  struct hostent *hent;
  int k;
  char *ptr, *tmp, *tmp2;

  owl_message_init(m);
  
  owl_message_set_type_zephyr(m);
  owl_message_set_direction_in(m);
  
  /* first save the full notice */
  memcpy(&(m->notice), n, sizeof(ZNotice_t));

  /* a little gross, we'll reaplace \r's with ' ' for now */
  owl_zephyr_hackaway_cr(&(m->notice));
  
  m->delete=0;

  /* set other info */
  owl_message_set_sender(m, n->z_sender);
  owl_message_set_class(m, n->z_class);
  owl_message_set_instance(m, n->z_class_inst);
  owl_message_set_recipient(m, n->z_recipient);
  if (n->z_opcode) {
    owl_message_set_opcode(m, n->z_opcode);
  } else {
    owl_message_set_opcode(m, "");
  }
  owl_message_set_zsig(m, n->z_message);

  if ((ptr=strchr(n->z_recipient, '@'))!=NULL) {
    owl_message_set_realm(m, ptr+1);
  } else {
    owl_message_set_realm(m, owl_zephyr_get_realm());
  }

  /* Set the "isloginout" attribute if it's a login message */
  if (!strcasecmp(n->z_class, "login") || !strcasecmp(n->z_class, OWL_WEBZEPHYR_CLASS)) {
    if (!strcasecmp(n->z_opcode, "user_login")) {
      owl_message_set_islogin(m);
    } else if (!strcasecmp(n->z_opcode, "user_logout")) {
      owl_message_set_islogout(m);
    }
  }

  /* is the "isprivate" attribute if it's a private zephyr */
  if (!strcasecmp(n->z_recipient, owl_zephyr_get_sender())) {
    owl_message_set_isprivate(m);
  }

  m->zwriteline=strdup("");

  /* set the body */
  ptr=owl_zephyr_get_message(n, &k);
  tmp=owl_malloc(k+10);
  memcpy(tmp, ptr, k);
  tmp[k]='\0';
  if (owl_global_is_newlinestrip(&g)) {
    tmp2=owl_util_stripnewlines(tmp);
    owl_message_set_body(m, tmp2);
    owl_free(tmp2);
  } else {
    owl_message_set_body(m, tmp);
  }
  owl_free(tmp);

#ifdef OWL_ENABLE_ZCRYPT
  /* if zcrypt is enabled try to decrypt the message */
  if (owl_global_is_zcrypt(&g) && !strcasecmp(n->z_opcode, "crypt")) {
    char *out;
    int ret;

    out=owl_malloc(strlen(owl_message_get_body(m))*16+20);
    ret=owl_zcrypt_decrypt(out, owl_message_get_body(m), owl_message_get_class(m), owl_message_get_instance(m));
    if (ret==0) {
      owl_message_set_body(m, out);
    } else {
      owl_free(out);
    }
  }
#endif  

  /* save the hostname */
  owl_function_debugmsg("About to do gethostbyaddr");
  hent=gethostbyaddr((char *) &(n->z_uid.zuid_addr), sizeof(n->z_uid.zuid_addr), AF_INET);
  if (hent && hent->h_name) {
    strcpy(m->hostname, hent->h_name);
  } else {
    strcpy(m->hostname, inet_ntoa(n->z_sender_addr));
  }

  /* save the time */
  m->time=owl_strdup(ctime((time_t *) &n->z_time.tv_sec));
  m->time[strlen(m->time)-1]='\0';
}
#else
void owl_message_create_from_znotice(owl_message *m, void *n)
{
}
#endif

void owl_message_create_from_zwriteline(owl_message *m, char *line, char *body, char *zsig)
{
  owl_zwrite z;
  int ret;
  
  owl_message_init(m);

  /* create a zwrite for the purpose of filling in other message fields */
  owl_zwrite_create_from_line(&z, line);

  /* set things */
  owl_message_set_direction_out(m);
  owl_message_set_type_zephyr(m);
  owl_message_set_sender(m, owl_zephyr_get_sender());
  owl_message_set_class(m, owl_zwrite_get_class(&z));
  owl_message_set_instance(m, owl_zwrite_get_instance(&z));
  if (owl_zwrite_get_numrecips(&z)>0) {
    owl_message_set_recipient(m,
			      long_zuser(owl_zwrite_get_recip_n(&z, 0))); /* only gets the first user, must fix */
  }
  owl_message_set_opcode(m, owl_zwrite_get_opcode(&z));
  owl_message_set_realm(m, owl_zwrite_get_realm(&z)); /* also a hack, but not here */
  m->zwriteline=owl_strdup(line);
  owl_message_set_body(m, body);
  owl_message_set_zsig(m, zsig);
  
  /* save the hostname */
  ret=gethostname(m->hostname, MAXHOSTNAMELEN);
  if (ret) {
    strcpy(m->hostname, "localhost");
  }

  owl_zwrite_free(&z);
}


void owl_message_pretty_zsig(owl_message *m, char *buff)
{
  /* stick a one line version of the zsig in buff */
  char *ptr;

  strcpy(buff, owl_message_get_zsig(m));
  ptr=strchr(buff, '\n');
  if (ptr) ptr[0]='\0';
}

void owl_message_free(owl_message *m)
{
  int i, j;
  owl_pair *p;
#ifdef HAVE_LIBZEPHYR    
  if (owl_message_is_type_zephyr(m) && owl_message_is_direction_in(m)) {
    ZFreeNotice(&(m->notice));
  }
#endif
  if (m->time) owl_free(m->time);
  if (m->zwriteline) owl_free(m->zwriteline);

  /* free all the attributes */
  j=owl_list_get_size(&(m->attributes));
  for (i=0; i<j; i++) {
    p=owl_list_get_element(&(m->attributes), i);
    owl_free(owl_pair_get_key(p));
    owl_free(owl_pair_get_value(p));
    owl_free(p);
  }

  owl_list_free_simple(&(m->attributes));
 
  owl_fmtext_free(&(m->fmtext));
}


