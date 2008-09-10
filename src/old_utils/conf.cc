/*
 *  Copyright (C) 2007  CZ.NIC, z.s.p.o.
 *
 *  This file is part of FRED.
 *
 *  FRED is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, version 2 of the License.
 *
 *  FRED is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with FRED.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "log.h"
#include "log/logger.h"

#define MAX_LINE 256
#define KEY_MAX 30

#include "conf.h"

// debug print
#ifdef DEBUG
#define debug printf
#else
#define debug /* printf */
#endif

#ifdef XMLCONF

int compare( const xmlChar * xml_value , char* value )
{
  if( xml_value == NULL ) return -1;
  else return strcmp( (char *) xml_value , value );
}

void Conf::get_element_names(xmlNode * a_node)
{
  xmlNode *cur_node = NULL;

  for(cur_node = a_node; cur_node; cur_node = cur_node->next)
  {
    if(cur_node->type == XML_ELEMENT_NODE )
    {

      debug("node type: Element, name: %s\n", cur_node->name);
      debug("  Content: %s\n", cur_node->children->content );
      debug("  Parent: Element, name: %s\n", cur_node->parent->name);

      if( compare( cur_node->parent->name , "DATABASE" ) ==0 )
      {
        if( compare( cur_node->name , "dbname" ) == 0 )
        {
          strcpy( dbname , (char *) cur_node->children->content );
        }
        else if( compare( cur_node->name , "user" ) == 0 )
        {
          strcpy( user , (char *)cur_node->children->content );
        }
        else if( compare( cur_node->name , "password" ) == 0 )
        {
          strcpy( password , (char *)cur_node->children->content );
        }
        else if( compare( cur_node->name , "host" ) == 0 )
        {
          strcpy( host , (char *) cur_node->children->content );
        }
        else if( compare( cur_node->name , "port" ) == 0 )
        {
          strcpy( port , (char *) cur_node->children->content );
        }
      }
      else if( compare( cur_node->parent->name , "SYSLOG" ) ==0 )
      {
        if( compare( cur_node->name , "level" ) == 0 )
        {
          log_level = GetLevel( (char *) cur_node->children->content );
        }
        else if( compare( cur_node->name , "local" ) == 0 )
        {
          log_local = GetLocal( (char *)cur_node->children->content);
        }
      }
      else if( compare( cur_node->parent->name , "NAMESERVICE" ) ==0 )
      {
        if( compare( cur_node->name , "ior" ) == 0 )
        {
          nameServiceIOR = (char *) cur_node->children->content;
        }
      }
      get_element_names(cur_node->children);
    }
  }

  bool Conf::ReadConfigFileXML(const char *filename )
  {
    xmlDoc *doc = NULL;
    xmlNode *root_element = NULL;
    bool ret;
    //parse the file and get the DOM 
    doc = xmlReadFile(filename , NULL, 0);

    if (doc == NULL) {
      LOG(ERROR_LOG, "could not parse file %s\n", filename );
      return false;
    }

    //Get the root element node 
    root_element = xmlDocGetRootElement(doc);

    debug("ROOT Element, name: %s\n", root_element->name);

    if( compare( root_element->name , "ccReg" ) == 0 )
    {
      get_element_names(root_element);
      ret =true;
    }
    else
    {
      LOG(ERROR_LOG, "bad root element  %s\n", root_element->name);
      ret = false;
    }
    //free the document 
    xmlFreeDoc(doc);

    //Free the global variables that may
    //have been allocated by the parser.

    xmlCleanupParser();

    return ret;
  }
#else 

// parse TXT config

int trim(
  char *Str)
{
  int i, j, len;

  len = strlen(Str);

  if (len > 0) {
    for (i = len -1; i > 0; i --) {
      if (Str[i] == ' ' || Str[i] == '\n' || Str[i] == '\r')
        Str[i] = 0;
      else
        break;
    }
  }

  len = strlen(Str);

  for (i = 0; i < len; i ++) {
    if (Str[i] > ' ')
      break;
  }

  for (j = 0; j < len-i; j ++) {
    Str[j] = Str[i+j];
  }
  Str[j] = 0;

  len = strlen(Str);
  return len;
}

bool Conf::ReadConfigFileTXT(
  const char *filename)
{
  FILE *f;
  char buf[MAX_LINE];
  char keys[MAX_KEYS][KEY_MAX] = { "dbname", "user", "password", "host",
      "port", "connect_timeout", "log_mask", "log_level", "log_local",
          "nameservice", "session_max", "session_wait", "docgen_path",
          "fileclient_path", "ebanka_url", "docgen_template_path",
          "nsset_level", "restricted_handles", "disable_epp_notifier",
          "session_registrar_max", "lock_epp_commands" };
  int key;
  char keyname[KEY_MAX];
  char value[MAX_LINE];
  int i, k, l, len, line;

  if ( (f = fopen(filename, "r") ) != NULL) {

    for (line=0;; line ++) {
      char *s = fgets(buf, MAX_LINE, f);
      if (feof(f) || (!s))
        break;

      value[0]=0;
      len = trim(buf); // trim bufefr
      if (buf[0] != '#' && len > 0) // neni komentar
      {

        // parse  config file
        for (i = 0, key = 0; i < len; i ++) {
          if (i >= KEY_MAX)
            break;
          if (buf[i] == '=') {
            keyname[i] = 0;
            for (k = 0; k < MAX_KEYS; k ++) {
              if (strncmp(keys[k], keyname, strlen(keys[k]) ) == 0) {
                key = k + 1;
                for (l = i +1; l < len; l ++)
                  if (buf[l] > ' ')
                    break;
                strcpy(value, buf + l);
                break;
              }
            }
            break;
          } else {
            if (buf[i] > ' ')
              keyname[i] = buf[i];
            else
              keyname[i] = 0;
          }
        }

        // debug print
        //   printf("config KEY %d %s value [%s]\n" , key , keyname  , value );

        switch (key) {
          case KEY_dbname:
            strcpy(dbname, value);
            break;
          case KEY_user:
            strcpy(user, value);
            break;
          case KEY_pass:
            strcpy(password, value);
            break;
          case KEY_host:
            strcpy(host, value);
            break;
          case KEY_port:
            strcpy(port, value);
            break;
          case KEY_timeout:
            strcpy(timeout, value);
            break;
          case KEY_session_wait:
            session_wait = atoi(value);
            break;
          case KEY_session_max:
            session_max = atoi(value);
            break;
          case KEY_log_level:
            log_level = atoi(value);
            break;
          case KEY_log_local:
            log_local = atoi(value);
            break;
          case KEY_nameservice:
            nameService = value;
            break;
          case KEY_docgen_path:
            docGenPath = value;
            break;
          case KEY_fileclient_path:
            fileClientPath = value;
            break;
          case KEY_ebanka_url:
            eBankaURL = value;
            break;
          case KEY_docgen_template_path:
            docGenTemplatePath = value;
            break;
          case KEY_nsset_level:
            nssetLevel = value;
            break;
          case KEY_restricted_handles:
            restrictedHandles = atoi(value);
            break;
          case KEY_disable_epp_notifier:
            disableEPPNotifier = atoi(value);
            break;
          case KEY_session_registrar_max:
            session_registrar_max = atoi(value);
            break;
          case KEY_lock_epp_commands:
            lockEPPCommands = atoi(value);
            break;
          default:
            LOG(ERROR_LOG, "parse error on line %d  [%s]\n" , line , buf );
            break;
        }

      }

    }

    fclose(f);
    return true;
  } else {
    LOG(ERROR_LOG, "Cannot open config file %s\n" , filename);
    return false;
  }

}
#endif

const char * Conf::GetDBconninfo()
{
  char buf[128];

  memset(conninfo, 0, 256);
  conninfo[0] = 0;

  if (strlen(dbname)) {
    sprintf(buf, "dbname=%s ", dbname);
    strcat(conninfo, buf) ;
  }
  if (strlen(user) ) {
    sprintf(buf, "user=%s ", user);
    strcat(conninfo, buf) ;
  }
  if (strlen(password)) {
    sprintf(buf, "password=%s ", password);
    strcat(conninfo, buf) ;
  }
  if (strlen(host) ) {
    sprintf(buf, "host=%s ", host);
    strcat(conninfo, buf) ;
  }
  if (strlen(port) ) {
    sprintf(buf, "port=%s ", port);
    strcat(conninfo, buf) ;
  }
  if (strlen(timeout) ) {
    sprintf(buf, "connect_timeout=%s ", timeout);
    strcat(conninfo, buf) ;
  }

  return conninfo;
}

int Conf::GetLocal(
  char *value)
{
  int l;

  l = atoi(value);
  if (l < 8)
    return l;

  return 0;
}

int Conf::GetLevel(
  char *value)
{
  char levels[8][8] = { "EMERG", "ALERT", "CRIT", "ERR", "WARNING", "NOTICE",
      "INFO", "DEBUG" };
  int i;
  int level=0;

  for (i = 0; i < 8; i ++) {
    if (strcmp(levels[i], value) == 0) {
      level = i+1;
      break;
    }
  }

  return level;
}
