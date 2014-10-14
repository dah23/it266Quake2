#include "g_local.h"
 
void Mark_Dummy_Spawn_Point(edict_t *ent)
{
ent->dummypointset=1;
VectorCopy(ent->s.origin,ent->dummypoint);
}
 
void dummydie(edict_t *ent, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point)
{
int n;
 
ent->think=G_FreeEdict;
ent->nextthink=level.time+0.1;
 
for (n= 0; n < 4; n++)
        ThrowGib (ent, "models/objects/gibs/sm_meat/tris.md2", damage, GIB_ORGANIC);
        ThrowGib (ent, "models/objects/gibs/chest/tris.md2", damage, GIB_ORGANIC);
        ThrowHead (ent, "models/objects/gibs/head2/tris.md2", damage, GIB_ORGANIC);
}
 
 
void dummypain (edict_t *ent, edict_t *other, float kick, int damage)
{
if (ent->health < (ent->max_health / 2))
        ent->s.skinnum = 1;
if (level.time < ent->pain_debounce_time)
        return;
ent->pain_debounce_time = level.time + 1;
/*ent->s.frame+=1;
if (ent->s.frame>5)
        ent->s.frame=0;
if (other->client)
        gi.cprintf(other,PRINT_HIGH,"%i health\n",ent->health);
*/
}
 
 
void Spawn_Dummy(edict_t *ent)
{
vec3_t  mins = {-16, -16, -24};
vec3_t  maxs = {16, 16, 32};
edict_t *dummy;
gclient_t *client = ent->client;
if (!(ent->dummypointset))
        return;
gi.cprintf(ent,PRINT_HIGH,"dummy being spawned");
 
 
dummy=G_Spawn();
dummy->s.modelindex=gi.modelindex ("models/monsters/insane/tris.md2");
dummy->s.skinnum=0;
dummy->s.frame=0;
dummy->solid=SOLID_NOT;
dummy->takedamage=DAMAGE_AIM;
dummy->movetype = MOVETYPE_WALK;
dummy->enemy=dummy;
dummy->classname="dummy";
dummy->die=dummydie;
dummy->pain=dummypain;
VectorCopy (mins, dummy->mins);
VectorCopy (maxs, dummy->maxs);
VectorClear (dummy->velocity);
dummy->s.effects = 0;
VectorCopy(ent->dummypoint,dummy->s.origin);
dummy->health=ent->health;
dummy->max_health=ent->max_health;
dummy->mass = 200;
dummy->flags = 0;
dummy->svflags &= ~SVF_DEADMONSTER;
VectorCopy (ent->s.angles, dummy->s.angles);
dummy->movetype = MOVETYPE_STEP;
dummy->s.origin[0] = client->ps.pmove.origin[0];
dummy->s.origin[1] = client->ps.pmove.origin[1]; 
dummy->s.origin[2] = client->ps.pmove.origin[2]; 
gi.linkentity (dummy);
 
}