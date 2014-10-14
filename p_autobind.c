   // QDeveLS functions
#include "g_local.h"    // Standard include header
#include "p_autobind.h" // AutoBind header
 
void CreateBind (edict_t *ent,char *key, char *command)
{
        // CreateBind binds the key "key" to the command "command" for the
        // client *ent;
        // * If required, add \" quote characters around commands and/or
        //   keys
 
        stuffcmd ("bind ",ent);
        stuffcmd (key,ent);
        stuffcmd (" ",ent);
        stuffcmd (command,ent);
        stuffcmd ("\n",ent);
}
 
 
void CreateAlias (edict_t *ent,char *name, char *commands)
{
        // CreateAlias - similar to "alias  ".  Sends to
        // client *ent;
 
        stuffcmd ("alias ",ent);
        stuffcmd (name,ent);
        stuffcmd (" ",ent);
        stuffcmd (commands,ent);
        stuffcmd ("\n",ent);
}