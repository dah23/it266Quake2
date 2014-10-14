#include "g_local.h"
#include "m_player.h"


#define POWER1_SOUND gi.soundindex("misc/power1.wav")
#define POWER2_SOUND gi.soundindex("misc/power2.wav")

// TYPE OF MONSTER ---- HEALTH
#define M_SOLDIERLT 0 // 20
#define M_SOLDIER 1 // 30
#define M_SOLDIERSS 2 // 40
#define M_FLIPPER 3 // 50
#define M_FLYER 4 // 50
#define M_INFANTRY 5 // 100
#define M_INSANE 6 // 100 - Crazy Marine
#define M_GUNNER 7 // 175
#define M_CHICK 8 // 175
#define M_PARASITE 9 // 175
#define M_FLOATER 10 // 200
#define M_HOVER 11 // 240
#define M_BERSERK 12 // 240
#define M_MEDIC 13 // 300
#define M_MUTANT 14 // 300
#define M_BRAIN 15 // 300
#define M_GLADIATOR 16 // 400
#define M_TANK 17 // 750
#define M_SUPERTANK 18 // 1500
#define M_BOSS2 19 // 2000
#define M_JORG 20 // 3000
#define M_MAKRON 21 // 3000

void SP_monster_jorg(edict_t *self);
void SP_monster_boss2(edict_t *self);
void SP_monster_berserk(edict_t *self);
void SP_monster_soldier_ss(edict_t *self);
void SP_monster_soldier(edict_t *self);
void SP_monster_brain(edict_t *self);
void SP_monster_chick(edict_t *self);
void SP_monster_flipper(edict_t *self);
void SP_monster_floater(edict_t *self);
void SP_monster_flyer(edict_t *self);
void SP_monster_gladiator(edict_t *self);
void SP_monster_hover(edict_t *self);
void SP_monster_infantry(edict_t *self);
void SP_misc_insane(edict_t *self);
void SP_monster_medic(edict_t *self);
void SP_monster_mutant(edict_t *self);
void SP_monster_parasite(edict_t *self);
void SP_monster_tank(edict_t *self);
void SP_monster_makron(edict_t *self);
void SP_monster_gunner(edict_t *self);
void SP_monster_supertank(edict_t *self);
void SP_monster_soldier_light(edict_t *self);

//======================================================
// Timer entity initiates the Monsters Self-Detonation.
//
// Monster = timer->activator
// Owner = timer->owner
//======================================================
void Monster_Explode(edict_t *timer) {
vec3_t zvec={0,0,0};

// Has Monster died already?
if (!timer->activator) {
G_FreeEdict(timer);
return; }

// Maximally damage the monster entity...
T_Damage(timer->activator, timer->owner, timer->owner, zvec, timer->activator->s.origin, NULL, 5000, 1, 0,
MOD_SPLASH);

// Spawn an explosion fireball..
G_Spawn_Explosion(TE_EXPLOSION2, timer->activator->s.origin, timer->activator->s.origin);

// Do grenade-type radius damage to anybody nearby
T_RadiusDamage(timer->activator, timer->owner, 40, NULL, 200, MOD_SPLASH);

G_FreeEdict(timer);
}

//=========================================================
void Spawn_Timer(edict_t *ent) {
edict_t *timer;

timer=G_Spawn();
timer->owner=ent;
ent->mynoise2=timer; // Link back to timer..
timer->activator=ent->goalentity;
timer->takedamage=DAMAGE_NO;
timer->movetype=MOVETYPE_NONE;
timer->solid = SOLID_NOT;
VectorClear(timer->s.origin);
VectorClear(timer->mins);
VectorClear(timer->maxs);
timer->think=Monster_Explode;
timer->nextthink=level.time + 180.0; // 3 mins to Self-Destruct
gi.linkentity(timer);
}

//=========================================================
qboolean Spawn_Monster(edict_t *ent, int mtype) {
edict_t *monster;
vec3_t forward, up, torigin;
int temp;

// See if we can project Monster forward 100 units...
AngleVectors(ent->s.angles, forward, NULL, up);
VectorCopy(ent->s.origin, torigin);
VectorMA(torigin, 100, forward, torigin);
if (gi.pointcontents(torigin) & MASK_SHOT) {
gi.cprintf(ent,PRINT_HIGH,"Cannot project into Solid!\n");
return false; }

// Create basic entity stuff...
monster = G_Spawn();
monster->classname = "XMonster"; // Used for Killed() & ClientObits()
monster->activator = ent; // Link back to Owner.
ent->goalentity = monster; // Link Ent to this Monster.
VectorCopy(torigin, monster->s.origin);

gi.linkentity(monster);

//
// Now the Monster Stuff..
//

skill->value=3; // Toggle Advanced AI Mode.

temp=deathmatch->value;
deathmatch->value=0; // Must = 0 to bypass quick-exit in Monster Code

// create this monster type
switch (mtype) {
case M_BERSERK: SP_monster_berserk(monster); break;
case M_BOSS2: SP_monster_boss2(monster); break;
case M_SOLDIERSS: SP_monster_soldier_ss(monster); break;
case M_JORG: SP_monster_jorg(monster); break;
case M_BRAIN: SP_monster_brain(monster); break;
case M_CHICK: SP_monster_chick(monster); break;
case M_FLIPPER: SP_monster_flipper(monster); break;
case M_FLOATER: SP_monster_floater(monster); break;
case M_FLYER: SP_monster_flyer(monster); break;
case M_INSANE: SP_misc_insane(monster); break;
case M_GLADIATOR: SP_monster_gladiator(monster); break;
case M_HOVER: SP_monster_hover(monster); break;
case M_INFANTRY: SP_monster_infantry(monster); break;
case M_SOLDIERLT: SP_monster_soldier_light(monster); break;
case M_SOLDIER: SP_monster_soldier(monster); break;
case M_MEDIC: SP_monster_medic(monster); break;
case M_MUTANT: SP_monster_mutant(monster); break;
case M_PARASITE: SP_monster_parasite(monster); break;
case M_TANK: SP_monster_tank(monster); break;
case M_MAKRON: SP_monster_makron(monster); break;
case M_GUNNER: SP_monster_gunner(monster); break;
case M_SUPERTANK: SP_monster_supertank(monster); break;
} // end switch

deathmatch->value=temp; // Restore to previous value.

monster->monsterinfo.aiflags &= AI_BRUTAL; // Kill Everything Mode!!

gi.sound(ent, CHAN_VOICE, POWER1_SOUND, 1, ATTN_IDLE, 0);

return true; // Spawn successful
}

//====================================================
void Cmd_Monsters_f(edict_t *ent) {

if (ent->goalentity) {
Monster_Explode(ent->mynoise2);
ent->goalentity=NULL;
gi.centerprintf(ent, "MONSTER DETONATED\n");
return; }

// Pass in the Flag for the Type of Monster you want!!
if (Spawn_Monster(ent, M_BRAIN)) {
Spawn_Timer(ent);
gi.centerprintf(ent, "MOVE AWAY NOW!!\n"); }
}