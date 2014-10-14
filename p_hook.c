#include "g_local.h"

#define ENTS_HOOKSTATE ent->client->hookstate
#define ENTS_HOOKTYPE ent->client->hooktype

#define HOOK_OWNERS_HOOKSTATE hook->owner->client->hookstate
#define HOOK_OWNERS_HOOKTYPE hook->owner->client->hooktype

// edict->sounds constants
#define MOTOR_OFF 0 // motor sound has not been triggered
#define MOTOR_START 1 // motor start sound has been triggered
#define MOTOR_ON 2 // motor running sound has been triggered

//==========================================================
void get_start_position(edict_t *ent, vec3_t start) {
vec3_t offset={0,0,0};
vec3_t forward={0,0,0};
vec3_t right={0,0,0};
vec3_t zvec={0,0,0};

// Get forward and right(direction) vectors
AngleVectors(ent->owner->client->v_angle, forward, right, NULL);

VectorSet(offset, 8, 8, ent->owner->viewheight-8);
// Add to the global offset of the world
VectorAdd(offset, zvec, offset);

// Get start vector
P_ProjectSource_Reverse(ent->owner->client, ent->owner->s.origin, offset, forward, right, start);
}

//==========================================================
void play_moving_chain_sound(edict_t *hook, qboolean chain_moving) {

// determine sound play if climbing or sliding
if (chain_moving)
switch (hook->sounds) {
case MOTOR_OFF:
// play start of chain climbing motor sound
gi.sound(hook->owner, CHAN_HOOK, HOOK_MOTOR1_SOUND, 1, ATTN_IDLE, 0);
hook->sounds = MOTOR_START;
break;
case MOTOR_START:
// play repetitive chain climbing sound
gi.sound(hook->owner, CHAN_HOOK, HOOK_MOTOR2_SOUND, 1, ATTN_IDLE, 0);
hook->sounds = MOTOR_ON;
break;
} // end switch
else
if (hook->sounds != MOTOR_OFF) {
gi.sound(hook->owner, CHAN_HOOK, HOOK_MOTOR3_SOUND, 1, ATTN_IDLE, 0);
hook->sounds = MOTOR_OFF; }
}

//==========================================================
void DropHook(edict_t *hook) {

HOOK_OWNERS_HOOKSTATE = HOOK_OFF;

gi.sound(hook->owner, CHAN_HOOK, HOOK_RETRACT_SOUND, 1, ATTN_IDLE, 0);

// removes hook
G_FreeEdict(hook);
}

//==========================================================
void Hook_Behavior(edict_t *hook) {
vec3_t zvec={0,0,0};
vec3_t start={0,0,0};
vec3_t chainvec={0,0,0}; // chain's vector
float chainlen; // length of extended chain
vec3_t velpart={0,0,0}; // player's velocity in relation to hook
float f1, f2, dprod; // restrainment forces
qboolean chain_moving=false;
int state;

state=HOOK_OWNERS_HOOKSTATE;

// Decide when to disconnect hook
// if hook is not ON OR
// if target is no longer solid OR
//(i.e. hook broke glass; exploded barrels, gibs)
// if player has died OR
// if player goes through teleport

if ((!(state & HOOK_ON))
||(hook->enemy->solid == SOLID_NOT)
||(hook->owner->deadflag)
||(hook->owner->s.event == EV_PLAYER_TELEPORT)){
DropHook(hook);
return; }

// gives hook same velocity as the entity it stuck to
VectorCopy(hook->enemy->velocity, hook->velocity);

// Grow the length of the chain
if (state & GROW_ON) {
chain_moving = true;
hook->angle += 20; }
else
// Shrink the length of the chain to Minimum
if (state & SHRINK_ON)
if (hook->angle > 10) {
chain_moving = true;
hook->angle -= 60;}
if (hook->angle < 10) {
hook->angle = 10;
chain_moving = false;}

// Make some chain sound occur..
play_moving_chain_sound(hook, chain_moving);
chain_moving=false; // chain not always moving..

//================
// chain physics
//================

// Get chain start position
get_start_position(hook, start);

VectorSubtract(hook->s.origin, start, chainvec); // get chain's vector
chainlen = VectorLength(chainvec); // get Chain Length at Vector

f1=0; // default state

// if player's location is beyond the chain's reach
if (chainlen > hook->angle) {
// determine player's velocity component of chain vector
dprod=DotProduct(hook->owner->velocity, chainvec)/DotProduct(chainvec, chainvec);
VectorScale(chainvec, dprod, velpart);
// restrainment default force
f2 =(chainlen - hook->angle)*5;
// if player's velocity heading is away from the hook
if (DotProduct(hook->owner->velocity, chainvec) < 0) {
// if chain has streched for 25 units
if (chainlen > hook->angle + 25)
// remove player's velocity component moving away from hook
VectorSubtract(hook->owner->velocity, velpart, hook->owner->velocity);
f1 = f2; }
else
// if player's velocity heading is towards the hook
if (VectorLength(velpart) < f2)
f1 = f2 - VectorLength(velpart);
} // end if

// applies chain restrainment
VectorNormalize(chainvec);
VectorMA(hook->owner->velocity, f1, chainvec, hook->owner->velocity);

if (hook->owner->client->hooktype == GRAPPLE_STYLE)
G_Spawn_Models(TE_GRAPPLE_CABLE,(short)(hook-g_edicts), hook->owner->s.origin, hook->s.origin, zvec, hook->s.origin);
else
G_Spawn_Trails(TE_BFG_LASER, start, hook->s.origin, hook->s.origin);

// set next think time
hook->nextthink = PRESENT_TIME + FRAMETIME;
}

//==========================================================
void Hook_Airborne(edict_t *hook) {
vec3_t zvec={0,0,0},start={0,0,0};
int state;

state=HOOK_OWNERS_HOOKSTATE;

// if hook not ON then exit..
if (!(state & HOOK_ON)) {
DropHook(hook);
return; }

get_start_position(hook, start);

if (hook->owner->client->hooktype == GRAPPLE_STYLE)
G_Spawn_Models(TE_GRAPPLE_CABLE,(short)(hook-g_edicts), hook->owner->s.origin, hook->s.origin, zvec, hook->s.origin);
else
G_Spawn_Trails(TE_BFG_LASER, start, hook->s.origin, hook->s.origin);

hook->nextthink = PRESENT_TIME+FRAMETIME;
}

//==========================================================
void Hook_Touch(edict_t *hook, edict_t *other, cplane_t *plane, csurface_t *surf){
vec3_t start={0,0,0}; // chain's start vector
vec3_t chainvec={0,0,0}; // chain's end vector
float chainlen;
int state;

state=HOOK_OWNERS_HOOKSTATE;

// if hook not ON or been fired at self then exit..
if (!(state & HOOK_ON) ||(hook == other)) {
DropHook(hook);
return; }

// Get chain start position
get_start_position(hook, start);

VectorSubtract(hook->s.origin,start,chainvec);
chainlen = VectorLength(chainvec);
// member angle is used to store the length of the chain
hook->angle = chainlen;

switch (other->solid) {
case SOLID_BBOX:
if (other->client) {
gi.sound(hook, CHAN_VOICE, HOOK_SMACK_SOUND, 1, ATTN_IDLE, 0);
// Show spurts of blood upon impact with player.
G_Spawn_Sparks(TE_BLOOD, hook->s.origin, plane->normal, hook->s.origin);}
break;
case SOLID_BSP:
// Show some sparks upon impact.
G_Spawn_Sparks(TE_SPARKS, hook->s.origin, plane->normal, hook->s.origin);
// Play chain hit 'clink'
gi.sound(hook, CHAN_VOICE, HOOK_HIT_SOUND, 1, ATTN_IDLE, 0);
// Clear the velocity vector
VectorClear(hook->avelocity);
break;
} // end switch

// inflict damage on damageable items
if (other->takedamage)
T_Damage(other, hook, hook->owner, hook->velocity, hook->s.origin, plane->normal, hook->dmg, 100, 0, MOD_HIT);

// hook gets the same velocity as the item it attached to
VectorCopy(other->velocity, hook->velocity);

// Automatic hook pulling upon contact.
HOOK_OWNERS_HOOKSTATE |= SHRINK_ON;

hook->enemy = other;
hook->touch = NULL;
hook->think = Hook_Behavior;

hook->nextthink = PRESENT_TIME + FRAMETIME;
}

//==========================================================
void Fire_Grapple_Hook(edict_t *ent) {
edict_t *hook=NULL;
vec3_t offset={0,0,0}, start={0,0,0};
vec3_t forward={0,0,0}, right={0,0,0};
int *hookstate=NULL;

hookstate = &ENTS_HOOKSTATE;
*hookstate = HOOK_ON;

// derive point of hook origin
AngleVectors(ENTS_V_ANGLE, forward, right, NULL);
VectorSet(offset, 8, 8, ENTS_VIEW_HEIGHT-8);
P_ProjectSource_Reverse(ent->client, ENTS_S_ORIGIN, offset, forward, right, start);

// spawn hook hook
hook = G_Spawn();
VectorCopy(start, hook->s.origin);
VectorCopy(forward, hook->movedir);
vectoangles(forward, hook->s.angles);
VectorScale(forward, 1600, hook->velocity);
hook->movetype = MOVETYPE_FLY;
hook->clipmask = MASK_SHOT;
hook->solid = SOLID_BBOX;
VectorClear(hook->mins);
VectorClear(hook->maxs);
hook->owner = ent;
hook->dmg = 20; // 20 Units of damage at impact.
hook->sounds = MOTOR_OFF; // keeps track of motor chain sound played

if (hook->owner->client->hooktype == GRAPPLE_STYLE) {
hook->s.modelindex = gi.modelindex(HOOK_MODEL);
VectorSet(hook->avelocity,0,0,-2000); }

hook->touch = Hook_Touch; // Function to call upon impact
hook->think = Hook_Airborne; // Function to call while hook in air
hook->nextthink = PRESENT_TIME + FRAMETIME;

gi.linkentity(hook);
}


//==========================================================
void Cmd_Hook_f(edict_t *ent, char *cmd) {
char *s=gi.argv(1);
int *hookstate=NULL;

if (!G_ClientInGame(ent)) return;

// create intermediate value
hookstate = &ent->client->hookstate;

if ((!(*hookstate & HOOK_ON))
&& (Q_stricmp(s, "action") == 0)) {
Fire_Grapple_Hook(ent);
return; }

if (*hookstate & HOOK_ON) {
if (Q_stricmp(s, "action") == 0) {
*hookstate = HOOK_OFF; // release hook
return; }

// deactivate chain growth or shrink
if (Q_stricmp(s, "stop") == 0) {
*hookstate -= *hookstate &(GROW_ON | SHRINK_ON);
return; }

// activate chain growth
if (Q_stricmp(s, "grow") == 0) {
*hookstate |= GROW_ON;
*hookstate -= *hookstate & SHRINK_ON;
return; }

// activate chain shrinking
if (Q_stricmp(s, "shrink") == 0) {
*hookstate |= SHRINK_ON;
*hookstate -= *hookstate & GROW_ON; }
} // endif
}