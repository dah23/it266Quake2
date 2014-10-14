#include "g_local.h"
 
void Mark_Third_Spawn(edict_t *ent, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point)
{
	ent->player_thirdpointset= 1;
	VectorCopy(ent->s.origin,ent->third_point);
}	

void third_die(edict_t *ent, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point)
{
int n;
 
ent->think=G_FreeEdict;
ent->nextthink=level.time+0.1;
 
for (n= 0; n < 4; n++)
        ThrowGib (ent, "models/objects/gibs/sm_meat/tris.md2", damage, GIB_ORGANIC);
        ThrowGib (ent, "models/objects/gibs/chest/tris.md2", damage, GIB_ORGANIC);
        ThrowHead (ent, "models/objects/gibs/head2/tris.md2", damage, GIB_ORGANIC);
}

void third_pain(edict_t *ent, edict_t *other, float kick, int damage)
{
	if (ent->health < (ent->max_health / 2))
        ent->s.skinnum = 1;
	if (level.time < ent->pain_debounce_time)
        return;
}

void Spawn_player_third(edict_t *ent)
{
		vec3_t  mins = {-16, -16, -24};
		vec3_t  maxs = {16, 16, 32};
		edict_t *player_third;
 
	if (!(ent->thirdpointset))
			return;
	
 
	player_third=G_Spawn();
	player_third->s.modelindex=gi.modelindex ("models/monsters/insane/tris.md2");
	player_third->s.skinnum=0;
	player_third->s.frame=0;
	player_third->solid=SOLID_BBOX;
	player_third->takedamage=DAMAGE_AIM;
	player_third->movetype = MOVETYPE_STEP;
	player_third->enemy=player_third;
	player_third->classname="player_third";
	player_third->die=third_die;
	player_third->pain=third_pain;
	VectorCopy (mins, player_third->mins);
	VectorCopy (maxs, player_third->maxs);
	VectorClear (player_third->velocity);
	player_third->s.effects = 0;
	VectorCopy(ent->third_point,player_third->s.origin);
	player_third->health=ent->health;
	player_third->max_health=ent->max_health;
	player_third->mass = 200;
	player_third->flags = 0;
	player_third->svflags &= ~SVF_DEADMONSTER;
	gi.linkentity (player_third);
 
}

