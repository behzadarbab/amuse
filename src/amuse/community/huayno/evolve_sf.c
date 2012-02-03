#include <tgmath.h>
#include <stdio.h>
#include <stdlib.h>
#include "evolve.h"

static void drift_naive(struct sys s, DOUBLE etime); /* drift/extrap sys to itime*/
static void split(FLOAT dt, struct sys s, struct sys *slow, struct sys *fast);
void kdk(struct sys s1,struct sys s2, DOUBLE stime, DOUBLE etime, DOUBLE dt);
static void dkd(struct sys s1,struct sys s2, DOUBLE stime, DOUBLE etime, DOUBLE dt);
static struct sys join(struct sys s1,struct sys s2);


void evolve_split_pass(struct sys sys1,struct sys sys2, 
                         DOUBLE stime, DOUBLE etime, DOUBLE dt, int calc_timestep)
{
  struct sys slow=zerosys,fast=zerosys;
  clevel++;
  if(etime == stime ||  dt==0 || clevel>=MAXLEVEL) ENDRUN("timestep too small");
  if(calc_timestep) timestep(sys1, join(sys1,sys2), SIGN(dt));
//  if(calc_timestep) timestep(sys1, sys1, SIGN(dt));
  split((FLOAT) dt, sys1, &slow, &fast);
  if(fast.n==0) 
  {
    deepsteps++;
    simtime+=dt;
  }  
  if(fast.n>0) evolve_split_pass(fast, join(slow,sys2), stime, stime+dt/2, dt/2,0);
  if(slow.n>0) kdk(slow,sys2, stime, etime, dt);
  if(fast.n>0) evolve_split_pass(fast, join(slow,sys2), stime+dt/2, etime, dt/2,1);
  clevel--;
}

void evolve_split_naive(struct sys sys1,struct sys sys2, 
                          DOUBLE stime, DOUBLE etime, DOUBLE dt, int calc_timestep)
{
  struct sys slow=zerosys,fast=zerosys;
  clevel++;
  if(etime == stime ||  dt==0 || clevel>=MAXLEVEL) ENDRUN("timestep too small");
  if(calc_timestep) timestep(sys1, join(sys1,sys2), SIGN(dt));
//  if(calc_timestep) timestep(sys1, sys1, SIGN(dt));
  split((FLOAT) dt, sys1, &slow, &fast);
  if(fast.n==0) 
  {
    deepsteps++;
    simtime+=dt;
  }  
  if(slow.n>0) kick(slow, join(sys1,sys2), dt/2);
  if(fast.n>0) evolve_split_naive(fast, join(slow,sys2), stime, stime+dt/2, dt/2,0);
  if(slow.n>0) drift_naive(join(slow,sys2),stime+dt/2);
  if(fast.n>0) evolve_split_naive(fast, join(slow,sys2), stime+dt/2, etime, dt/2,1);
  if(slow.n>0) drift_naive(join(slow,sys2),etime);
  if(slow.n>0) kick(slow, join(sys1,sys2), dt/2);
  clevel--;
}

void evolve_split_bridge(struct sys s, DOUBLE stime, DOUBLE etime, DOUBLE dt, int calc_timestep)
{
  struct sys slow=zerosys,fast=zerosys;
  clevel++;
  if(etime == stime ||  dt==0 || clevel>=MAXLEVEL) ENDRUN("timestep too small");
  if(calc_timestep) timestep(s,s, SIGN(dt));
  split((FLOAT) dt, s, &slow, &fast);
  if(fast.n==0) 
  {
    deepsteps++;
    simtime+=dt;
  }  
  if(slow.n>0 && fast.n>0) kick(slow, fast, dt/2);
  if(slow.n>0 && fast.n>0) kick(fast, slow, dt/2);
  if(fast.n>0) evolve_split_bridge(fast, stime, stime+dt/2, dt/2,0); /* note calc_timestep? */
  if(slow.n>0) kdk(slow,zerosys, stime, etime, dt);
  if(fast.n>0) evolve_split_bridge(fast, stime+dt/2, etime, dt/2,1);
  if(slow.n>0 && fast.n>0) kick(slow, fast, dt/2);
  if(slow.n>0 && fast.n>0) kick(fast, slow, dt/2);
  clevel--;
}

void evolve_split_bridge_dkd(struct sys s, DOUBLE stime, DOUBLE etime, DOUBLE dt, int calc_timestep)
{
  struct sys slow=zerosys,fast=zerosys;
  clevel++;
  if(etime == stime ||  dt==0 || clevel>=MAXLEVEL) ENDRUN("timestep too small");
  if(calc_timestep) timestep(s,s, SIGN(dt));
  split((FLOAT) dt, s, &slow, &fast);
  if(fast.n==0) 
  {
    deepsteps++;
    simtime+=dt;
  }  
  if(slow.n>0 && fast.n>0) kick(slow, fast, dt/2);
  if(slow.n>0 && fast.n>0) kick(fast, slow, dt/2);
  if(fast.n>0) evolve_split_bridge_dkd(fast, stime, stime+dt/2, dt/2,0); /* note calc_timestep? */
  if(slow.n>0) dkd(slow,zerosys, stime, etime, dt);
  if(fast.n>0) evolve_split_bridge_dkd(fast, stime+dt/2, etime, dt/2,1);
  if(slow.n>0 && fast.n>0) kick(slow, fast, dt/2);
  if(slow.n>0 && fast.n>0) kick(fast, slow, dt/2);
  clevel--;
}

void evolve_split_hold(struct sys s, DOUBLE stime, DOUBLE etime, DOUBLE dt, int calc_timestep)
{
  struct sys slow=zerosys,fast=zerosys;
  clevel++;
  if(etime == stime ||  dt==0 || clevel>=MAXLEVEL) ENDRUN("timestep too small");
  if(calc_timestep) timestep(s,s, SIGN(dt));
  split((FLOAT) dt, s, &slow, &fast);
  if(fast.n==0) 
  {
    deepsteps++;
    simtime+=dt;
  }  
  if(fast.n>0) evolve_split_hold(fast, stime, stime+dt/2, dt/2,0);
  if(slow.n>0) kdk(slow,fast,stime,etime,dt);
  if(fast.n>0) evolve_split_hold(fast, stime+dt/2, etime, dt/2,1);
  clevel--;
}

void evolve_split_hold_dkd(struct sys s, DOUBLE stime, DOUBLE etime, DOUBLE dt, int calc_timestep)
{
  struct sys slow=zerosys,fast=zerosys;
  clevel++;
  if(etime == stime ||  dt==0 || clevel>=MAXLEVEL) ENDRUN("timestep too small");
  if(calc_timestep) timestep(s,s, SIGN(dt));
  split((FLOAT) dt, s, &slow, &fast);
  if(fast.n==0) 
  {
    deepsteps++;
    simtime+=dt;
  }  
  if(fast.n>0) evolve_split_hold_dkd(fast, stime, stime+dt/2, dt/2,0);
  if(slow.n>0) dkd(slow,fast,stime,etime,dt);
  if(fast.n>0) evolve_split_hold_dkd(fast, stime+dt/2, etime, dt/2,1);
  clevel--;
}

void evolve_split_pass_dkd(struct sys sys1,struct sys sys2, 
                         DOUBLE stime, DOUBLE etime, DOUBLE dt, int calc_timestep)
{
  struct sys slow=zerosys,fast=zerosys;
  clevel++;
  if(etime == stime ||  dt==0 || clevel>=MAXLEVEL) ENDRUN("timestep too small");
  if(calc_timestep) timestep(sys1, join(sys1,sys2), SIGN(dt));
//  if(calc_timestep) timestep(sys1, sys1, SIGN(dt));
  split((FLOAT) dt, sys1, &slow, &fast);
  if(fast.n==0) 
  {
    deepsteps++;
    simtime+=dt;
  }  
  if(fast.n>0) evolve_split_pass_dkd(fast, join(slow,sys2), stime, stime+dt/2, dt/2,0);
  if(slow.n>0) dkd(slow,sys2, stime, etime, dt);
  if(fast.n>0) evolve_split_pass_dkd(fast, join(slow,sys2), stime+dt/2, etime, dt/2,1);
  clevel--;
}

void evolve_split_ppass_dkd(struct sys sys1,struct sys sys2, 
                         DOUBLE stime, DOUBLE etime, DOUBLE dt, int calc_timestep)
{
  struct sys slow=zerosys,fast=zerosys;
  clevel++;
  if(etime == stime ||  dt==0 || clevel>=MAXLEVEL) ENDRUN("timestep too small");
  if(calc_timestep) timestep(sys1, join(sys1,sys2), SIGN(dt));
//  if(calc_timestep) timestep(sys1, sys1, SIGN(dt));
  split((FLOAT) dt, sys1, &slow, &fast);
  if(fast.n==0) 
  {
    deepsteps++;
    simtime+=dt;
  }  
  if(fast.n>0) 
    evolve_split_ppass_dkd(fast, join(slow,sys2), stime, stime+dt/2, dt/2,0);
  else  
    drift(join(slow,sys2),etime,dt/2);
  if(slow.n>0) kick(slow,join(slow,sys2), dt);
  if(slow.n>0) kick(sys2,slow, dt);
  if(fast.n>0) 
    evolve_split_ppass_dkd(fast, join(slow,sys2), stime+dt/2, etime, dt/2,1);
  else  
    drift(join(slow,sys2),etime,dt/2);
  clevel--;
}


static void drift_naive(struct sys s, DOUBLE etime)
{
  UINT i;
  DOUBLE dt;
  for(i=0;i<s.n;i++)
  {
    dt=etime-s.part[i].postime;
    s.part[i].pos[0]+=dt*s.part[i].vel[0];
    s.part[i].pos[1]+=dt*s.part[i].vel[1];
    s.part[i].pos[2]+=dt*s.part[i].vel[2];
    s.part[i].postime=etime;
    s.part[i].timestep=HUGE_VAL;
  }
  dstep[clevel]++;
  dcount[clevel]+=s.n;
}

#define K1   ( (14-sqrt((DOUBLE) 19))/108 )
#define K2   ( (20-7*sqrt((DOUBLE) 19))/108 )
#define K3   ( (((DOUBLE) 1)/2)-(K1+K2) )
#define D1   ( ((DOUBLE) 2)/5 )
#define D2   ( -((DOUBLE) 1)/10 )
#define D3   ( 1-(2*D1+2*D2) )
#define N1   4
void evolve_sf_4m5(struct sys s, DOUBLE stime, DOUBLE etime, DOUBLE dt, int calc_timestep)
{
  int i;
  struct sys slow=zerosys,fast=zerosys;
  clevel++;
  if(etime == stime ||  dt==0 || clevel>=MAXLEVEL) ENDRUN("timestep too small");
  if(calc_timestep) timestep(s,s, SIGN(dt));
  split((FLOAT) dt, s, &slow, &fast);
  if(fast.n==0) 
  {
    deepsteps++;
    simtime+=dt;
    timetrack+=fabs(dt);
  }  
  if(slow.n>0) kick(slow,join(fast,slow), K1*dt);
  if(fast.n>0 && slow.n>0) kick(fast,slow, K1*dt);

  if(slow.n>0) drift(slow,stime+D1*dt,D1*dt);
  if(fast.n>0) 
    for(i=0;i<N1;i++)
    {
      evolve_sf_4m5(fast,stime,stime+D1*dt/N1,D1*dt/N1,i==0?0:1);
      stime+=D1*dt/N1; 
    }
  if(slow.n>0) kick(slow,join(fast,slow), K2*dt);
  if(fast.n>0 && slow.n>0) kick(fast,slow, K2*dt);

  if(slow.n>0) drift(slow,stime+D2*dt,D2*dt);
  if(fast.n>0) evolve_sf_4m5(fast,stime,stime+D2*dt,D2*dt,1);
  stime+=D2*dt;

  if(slow.n>0) kick(slow,join(fast,slow), K3*dt);
  if(fast.n>0 && slow.n>0) kick(fast,slow, K3*dt);

  if(slow.n>0) drift(slow,stime+D3*dt,D3*dt);
  if(fast.n>0) 
    for(i=0;i<N1;i++)
    {
      evolve_sf_4m5(fast,stime,stime+D3*dt/N1,D3*dt/N1,1);
      stime+=D3*dt/N1;
    }
  if(slow.n>0) kick(slow,join(fast,slow), K3*dt);
  if(fast.n>0 && slow.n>0) kick(fast,slow, K3*dt);

  if(slow.n>0) drift(slow,stime+D2*dt,D2*dt);
  if(fast.n>0) evolve_sf_4m5(fast,stime,stime+D2*dt,D2*dt,1);
  stime+=D2*dt;

  if(slow.n>0) kick(slow,join(fast,slow), K2*dt);
  if(fast.n>0 && slow.n>0) kick(fast,slow, K2*dt);

  if(slow.n>0) drift(slow,etime,D1*dt);
  if(fast.n>0) 
    for(i=0;i<N1;i++)
    {
      evolve_sf_4m5(fast,stime,i==N1-1?etime:stime+D1*dt/N1,D1*dt/N1,1);
      stime+=D1*dt/N1; 
    }
  if(slow.n>0) kick(slow,join(fast,slow), K1*dt);
  if(fast.n>0 && slow.n>0) kick(fast,slow, K1*dt);    
  clevel--;
}
#undef K1
#undef K2
#undef K3
#undef D1
#undef D2
#undef D3
#undef N1

#define K1   ( (642+sqrt( (DOUBLE) 471 ))/3924 )
#define K2   (  121*(12- sqrt( (DOUBLE) 471 ) )/3924 )
#define K3   (  1-2*(K1+K2) )
#define D1   ( ((DOUBLE) 6)/11 )
#define D2   ( ((DOUBLE) 0.5)-D1 ) 
#define N1   12
void evolve_sf_4m4(struct sys s, DOUBLE stime, DOUBLE etime, DOUBLE dt, int calc_timestep)
{
  int i;
  struct sys slow=zerosys,fast=zerosys;
  clevel++;
  if(etime == stime ||  dt==0 || clevel>=MAXLEVEL) ENDRUN("timestep too small");
  if(calc_timestep) timestep(s,s, SIGN(dt));
  split((FLOAT) dt, s, &slow, &fast);
  if(fast.n==0) 
  {
    deepsteps++;
    simtime+=dt;
    timetrack+=fabs(dt);
  }  
  if(slow.n>0) kick(slow,join(fast,slow), K1*dt);
  if(fast.n>0 && slow.n>0) kick(fast,slow, K1*dt);

  if(slow.n>0) drift(slow,stime+D1*dt,D1*dt);
  if(fast.n>0) 
    for(i=0;i<N1;i++)
    {
      evolve_sf_4m5(fast,stime,stime+D1*dt/N1,D1*dt/N1,i==0?0:1);
      stime+=D1*dt/N1; 
    }
  if(slow.n>0) kick(slow,join(fast,slow), K2*dt);
  if(fast.n>0 && slow.n>0) kick(fast,slow, K2*dt);

  if(slow.n>0) drift(slow,stime+D2*dt,D2*dt);
  if(fast.n>0) evolve_sf_4m5(fast,stime,stime+D2*dt,D2*dt,1);
  stime+=D2*dt;

  if(slow.n>0) kick(slow,join(fast,slow), K3*dt);
  if(fast.n>0 && slow.n>0) kick(fast,slow, K3*dt);

  if(slow.n>0) drift(slow,stime+D2*dt,D2*dt);
  if(fast.n>0) evolve_sf_4m5(fast,stime,stime+D2*dt,D2*dt,1);
  stime+=D2*dt;

  if(slow.n>0) kick(slow,join(fast,slow), K2*dt);
  if(fast.n>0 && slow.n>0) kick(fast,slow, K2*dt);

  if(slow.n>0) drift(slow,etime,D1*dt);
  if(fast.n>0) 
    for(i=0;i<N1;i++)
    {
      evolve_sf_4m5(fast,stime,i==N1-1?etime:stime+D1*dt/N1,D1*dt/N1,1);
      stime+=D1*dt/N1; 
    }
  if(slow.n>0) kick(slow,join(fast,slow), K1*dt);
  if(fast.n>0 && slow.n>0) kick(fast,slow, K1*dt);    
  clevel--;
}
#undef K1
#undef K2
#undef K3
#undef D1
#undef D2


static void split(FLOAT dt, struct sys s, struct sys *slow, struct sys *fast)
{
  UINT i=0;
  struct particle *left, *right;
  left=s.part;
  right=s.last;
  dt=fabs(dt);
  while(1)
  {
    if(i>=s.n) ENDRUN("split error 1");
    i++;
    while(left->timestep<dt && left<right) left++;
    while(right->timestep>=dt && left<right) right--;
    if(left<right) 
      {SWAP( *left, *right, struct particle);}
    else 
      break;  
  }
  if(left->timestep<dt) left++;
  slow->n=s.last-left+1;
  fast->n=(left-s.part);  
  if(fast->n==1)
  {
    fast->n=0;
    slow->n=s.n;
  } 
  if(slow->n > 0)
  {
    slow->part=s.part+fast->n;
    slow->last=s.last;//slow->part+slow->n-1;
  }
  if(fast->n > 0)
  {
    fast->part=s.part;
    fast->last=s.part+fast->n-1;
  }
  if(fast->n+slow->n !=s.n) ENDRUN( "split error 2");
  for(i=0;i<s.n;i++) s.part[i].level=clevel;
}

static struct sys join(struct sys s1,struct sys s2)
{
  struct sys s=zerosys;
  if(s1.n == 0) return s2;
  if(s2.n == 0) return s1;  
  s.n=s1.n+s2.n;
  if(s1.part+s1.n == s2.part)
  {
    s.part=s1.part;
    s.last=s2.last;
  } else
  {
    if(s2.part+s2.n == s1.part)
    {
      s.part=s2.part;
      s.last=s1.last;
    } else
      ENDRUN("join error 1");
  }   
  if(s.last-s.part + 1 != s.n) ENDRUN("join error 2");
  return s;
}

void kdk(struct sys s1,struct sys s2, DOUBLE stime, DOUBLE etime, DOUBLE dt)
{
  if(s2.n>0) kick(s2, s1, dt/2);
  kick(s1,join(s1,s2),dt/2);
  drift(s1,etime, dt);
  kick(s1,join(s1,s2),dt/2);
  if(s2.n>0) kick(s2, s1, dt/2);
}

static void dkd(struct sys s1,struct sys s2, DOUBLE stime, DOUBLE etime, DOUBLE dt)
{
  drift(s1,stime+dt/2, dt/2);
  kick(s1,join(s1,s2),dt);
  if(s2.n>0) kick(s2, s1, dt);
  drift(s1,etime, dt/2);
}