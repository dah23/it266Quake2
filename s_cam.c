#include "g_local.h"

void ChasecamTrack (edict_t *ent);
void ChasecamStart(edict_t *ent)
{
edict_t *chasecam;
ent->client->chasetoggle = 1;
ent->client->ps.gunindex=0; //removes gun model from view

chasecam = G_Spawn ();
chasecam->owner = ent;
chasecam->solid = SOLID_NOT;

chasecam->movetype = MOVETYPE_FLYMISSILE;
ent->client->ps.pmove.pm_flags |= PMF_NO_PREDICTION; //turns cam predicting off, making it smoother
ent->svflags |= SVF_NOCLIENT;

VectorCopy (ent->s.angles, chasecam->s.angles); //copy angles of player model to cam ent
VectorClear (chasecam->mins);
VectorClear (chasecam->maxs);

VectorCopy (ent->s.origin, chasecam->s.origin);

chasecam->classname = "chasecam";
chasecam->prethink = ChasecamTrack;
ent->client->chasecam = chasecam;
ent->client->oldplayer = G_Spawn();

}

void ChasecamRemove(edict_t *ent)
{
	if(ent->client->chasetoggle == 0)
	{
		ent->client->ps.gunindex = gi.modelindex(ent->client->pers.weapon->view_model);
		ent->s.modelindex = ent->client->oldplayer->s.modelindex;
		VectorClear(ent->client->chasecam->velocity);
		
		ent->svflags &= ~SVF_NOCLIENT;
		
		free(ent->client->oldplayer);
		G_FreeEdict(ent->client->oldplayer);
		G_FreeEdict(ent->client->chasecam);
	}
	else
	{
		ent->client->chasetoggle = 2;
	
	}
}

void ChasecamTrack(edict_t *ent)
{
	trace_t tr;
    vec3_t spot2, headorg, angle;
    vec3_t forward, right, up;
    float dist;
	
	ent->nextthink = level.time + 0.100;
	
	VectorCopy(ent->owner->s.origin, headorg);
	if(!(ent->owner->client->ps.pmove.pm_flags & PMF_DUCKED))
        headorg[2] += 25;
    else
		headorg[2] += 10;
		
	if(ent->owner->client->chasedist1 <= 0)
        ent->owner->client->chasedist1 = 80;
	
	VectorCopy(ent->owner->client->v_angle, angle);
	AngleVectors (ent->owner->client->ps.viewangles, forward, right, up);
	
	dist = ent->chasedist2 / ent->owner->client->chasedist1;
	
	VectorScale(forward, -ent->owner->client->chasedist1, spot2);
	spot2[2] += 5.00;
    VectorScale(spot2, dist, spot2);
	
	VectorAdd(headorg, spot2, spot2);
    tr = gi.trace (headorg, NULL, NULL, spot2, ent->owner, true);
    VectorSubtract(spot2, headorg, spot2);
	
	VectorScale(spot2, tr.fraction - 0.05, spot2); // Scale the distance if the trace hit a wall
 
    VectorAdd(spot2, headorg, spot2);
    VectorCopy(spot2, ent->s.origin);
    VectorCopy(angle, ent->s.angles);
	
	VectorCopy (ent->s.origin, ent->movedir);
	
	if(ent->owner->client->chasetoggle == 2)
	{
		ent->chasedist2 -= 6;
        if (ent->chasedist2 <= 0)
		{
			ent->chasedist2 = 0;
            ent->owner->client->chasetoggle = 0;
            ChasecamRemove(ent->owner);
		}
	}
	else if (ent->chasedist2 < ent->owner->client->chasedist1)
	{
		ent->chasedist2 += 6;
        if (ent->chasedist2 > ent->owner->client->chasedist1)
			ent->chasedist2 = ent->owner->client->chasedist1;
	}
		
}

void Cmd_Chasecam_Toggle (edict_t *ent) 
{
	if (ent->client->chasetoggle > 0)
		ChasecamRemove (ent);
    else
        ChasecamStart (ent);
}

void Cmd_Chasecam_Zoom(edict_t *ent, char *opt) 
{
	if(!strcmp(opt, "out") && ent->client->chasetoggle > 0)
		ent->client->chasedist1 += 10;
    else if(ent->client->chasetoggle > 0)
		ent->client->chasedist1 -= 10;
 
    if(ent->client->chasedist1 <= 0)
		ent->client->chasedist1 = 80;
 
    gi.bprintf(PRINT_HIGH, "Zoom Amount: %f\n", ent->client->chasedist1);

}

void Cmd_Chasecam_Viewlock(edict_t *ent) 
{
	if(ent->client->chasetoggle == 1)
		ent->client->chasetoggle = 3;
    else if(ent->client->chasetoggle == 3)
		ent->client->chasetoggle = 1;
    else if(ent->client->chasetoggle == 0)
	{
		ChasecamStart (ent);
        ent->client->chasetoggle = 3;
    }
	
}

void CheckChasecam_Viewent (edict_t *ent) 
{
	gclient_t *cl;
    if (!ent->client->oldplayer->client) 
	{
		cl = (gclient_t *)
        malloc(sizeof(gclient_t));
        ent->client->oldplayer->client = cl;
    }
	
	 if ((ent->client->chasetoggle >= 1) && (ent->client->oldplayer)) 
	 {
		ent->client->oldplayer->s.frame = ent->s.frame;
		VectorCopy (ent->s.origin, ent->client->oldplayer->s.origin);
        VectorCopy (ent->velocity, ent->client->oldplayer->velocity);
        VectorCopy (ent->s.angles, ent->client->oldplayer->s.angles);
		
		ent->client->oldplayer->s = ent->s;
		
		gi.linkentity (ent->client->oldplayer);
	}
}


