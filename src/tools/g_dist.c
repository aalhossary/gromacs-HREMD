/*
 *       $Id$
 *
 *       This source code is part of
 *
 *        G   R   O   M   A   C   S
 *
 * GROningen MAchine for Chemical Simulations
 *
 *            VERSION 2.0
 * 
 * Copyright (c) 1991-1997
 * BIOSON Research Institute, Dept. of Biophysical Chemistry
 * University of Groningen, The Netherlands
 * 
 * Please refer to:
 * GROMACS: A message-passing parallel molecular dynamics implementation
 * H.J.C. Berendsen, D. van der Spoel and R. van Drunen
 * Comp. Phys. Comm. 91, 43-56 (1995)
 *
 * Also check out our WWW page:
 * http://rugmd0.chem.rug.nl/~gmx
 * or e-mail to:
 * gromacs@chem.rug.nl
 *
 * And Hey:
 * GROningen MAchine for Chemical Simulation
 */
static char *SRCID_g_dist_c = "$Id$";

#include <typedefs.h>
#include "smalloc.h"
#include "macros.h"
#include "math.h"
#include "xvgr.h"
#include "copyrite.h"
#include "statutil.h"
#include "string2.h"
#include "vec.h"
#include "rdgroup.h"
#include "pbc.h"
#include "fatal.h"
#include "futil.h"
#include "gstat.h"

void main(int argc,char *argv[])
{
  static char *desc[] = {
    "g_dist calculates the distances between the center of masses of two",
    "groups defined by the index file"
  };
  
  t_topology *top=NULL;
  real t,dx,dy,dz;
  rvec *x=NULL,*v=NULL;
  matrix box;
  int status;
  int natoms;

  int g,d,i,teller=0;
  atom_id aid;

  int     ngrps;     /* the number of index groups */
  atom_id **index;   /* the index for the atom numbers */
  int     *isize;    /* the size of each group */
  char    **grpname; /* the name of each group */
  rvec    *com;
  real    *mass;
  FILE *fp=NULL;

  
  t_filenm fnm[] = {
    { efTPX, NULL, NULL, ffREAD },
    { efTRX, "-f", NULL, ffREAD },
    { efNDX, NULL, NULL, ffREAD },
    { efXVG, NULL, NULL, ffOPTWR },
  };
#define NFILE asize(fnm)


  CopyRight(stdout,argv[0]);

  parse_common_args(&argc,argv,PCA_CAN_TIME,TRUE,
		    NFILE,fnm,0,NULL,asize(desc),desc,0,NULL);
  

  top=read_top(ftp2fn(efTPX,NFILE,fnm));
  
  natoms=read_first_x(&status,ftp2fn(efTRX,NFILE,fnm),&t,&x,box);
  

  /* open output file */
  fp = xvgropen(ftp2fn(efXVG,NFILE,fnm),
		"Distance","Distance (nm)","Time (ps)");
  
  /* read index files */
  ngrps = 2;
  snew(com,ngrps);
  snew(grpname,ngrps);
  snew(index,ngrps);
  snew(isize,ngrps);
  rd_index(ftp2fn(efNDX,NFILE,fnm),ngrps,isize,index,grpname);
  
  /* calculate mass */
  snew(mass,ngrps);
  for(g=0;(g<ngrps);g++) {
    mass[g]=0;
    for(i=0;(i<isize[g]);i++) {
      mass[g]+=top->atoms.atom[index[g][i]].m;
    }
  }

  do {
    /* calculate center of masses */
    for(g=0;(g<ngrps);g++) {
      for(d=0;(d<DIM);d++) {
	com[g][d]=0;
	for(i=0;(i<isize[g]);i++) {
	  com[g][d] += x[index[g][i]][d] * top->atoms.atom[index[g][i]].m;
	}
	com[g][d] /= mass[g];
      }
    }
    
    /* write to output */
    fprintf(fp,"%8.3f ",t);
    for(g=0;(g<ngrps/2);g++) {
      dx=com[2 * g][XX] - com[2 * g + 1][XX];
      dy=com[2 * g][YY] - com[2 * g + 1][YY];
      dz=com[2 * g][ZZ] - com[2 * g + 1][ZZ];
      fprintf(fp,"%10.5f %10.5f %10.5f %10.5f",
	      sqrt(sqr(dx)+sqr(dy)+sqr(dz)),dx,dy,dz);
    }
    fprintf(fp,"\n");
    teller++;
  } while (read_next_x(status,&t,natoms,x,box));
  fclose(fp);
  fprintf(stderr,"\n");
  close_trj(status);
  
  thanx(stdout);
}
