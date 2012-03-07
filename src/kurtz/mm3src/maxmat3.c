/*
  Copyright (c) 2003 by Stefan Kurtz and The Institute for
  Genomic Research.  This is OSI Certified Open Source Software.
  Please see the file LICENSE for licensing information and
  the file ACKNOWLEDGEMENTS for names of contributors to the
  code base.
*/

//\IgnoreLatex{

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mpi.h>
#include "types.h"
#include "debugdef.h"
#include "errordef.h"
#include "protodef.h"
#include "spacedef.h"
#include "megabytes.h"
#include "maxmatdef.h"

#ifdef DEBUG

#define SHOWBOOLEANVALUE(CC)\
        fprintf(stderr,"# %s=%s\n",#CC,SHOWBOOL(mmcallinfo->CC));

static void showmaxmatflags (char *program,
                             MMcallinfo *mmcallinfo)
{
  Uint i;

  fprintf (stderr,"# %s called with the following flags\n", program);
  SHOWBOOLEANVALUE (showstring);
  SHOWBOOLEANVALUE (reversecomplement);
  SHOWBOOLEANVALUE (forward);
  SHOWBOOLEANVALUE (showreversepositions);
  SHOWBOOLEANVALUE (showsequencelengths);
  SHOWBOOLEANVALUE (matchnucleotidesonly);
  SHOWBOOLEANVALUE (cmumcand);
  SHOWBOOLEANVALUE (cmum);
  fprintf (stderr,"# minmatchlength=%lu\n",
	   (Showuint) mmcallinfo->minmatchlength);
  fprintf (stderr,"# subject-file=\"%s\"\n", &mmcallinfo->subjectfile[0]);
  for(i=0; i< mmcallinfo->numofqueryfiles; i++)
  {
    fprintf (stderr,"# query-file=\"%s\"\n", &mmcallinfo->queryfilelist[i][0]);
  }
}
#endif

//}

/*EE
  This module contains the main function of maxmatch3. It calls
  the following three functions in an appropriate order and with
  proper arguments.
*/

/*EE
  The following function is imported form \texttt{maxmatopt.c}.
*/

Sint parsemaxmatoptions (MMcallinfo *maxmatcallinfo,
                         int argc,
                         char **argv);

/*EE
  The following function is imported form \texttt{maxmatinp.c}.
*/

Sint getmaxmatinput (Multiseq *subjectmultiseq,
                     BOOL matchnucleotidesonly,
                     char *subjectfile);

/*EE
  The following function is imported form \texttt{procmaxmat.c}.
*/

Sint procmaxmatches(MMcallinfo *mmcallinfo,
                    Multiseq *subjectmultiseq);

//\IgnoreLatex{

/*
  This is the main function.
*/

using namespace std;

int main(int argc, char **argv)
{
  Sint retcode;
  MMcallinfo mmcallinfo;
  Multiseq subjectmultiseq;
    int numprocs, rank, namelen;
    double start, finish;

    MPI::Init(argc, argv);
    numprocs = MPI::COMM_WORLD.Get_size();
    rank = MPI::COMM_WORLD.Get_rank();

  DEBUGLEVELSET;
  initclock();
  retcode = parsemaxmatoptions (&mmcallinfo, argc, argv);
  if (retcode < 0)
  {
        fprintf(stderr,"%s: %s\n",argv[0],messagespace());
        MPI::Finalize();
        return EXIT_FAILURE;
  }
  if (retcode == 1)   // program was called with option -help
  {
    checkspaceleak ();
    mmcheckspaceleak ();
    MPI::Finalize();
    return EXIT_SUCCESS;
  }
  DEBUGCODE(1,showmaxmatflags (argv[0], &mmcallinfo));
    if (rank == 0) 
    {
        start = MPI::Wtime();
  if (getmaxmatinput (&subjectmultiseq,
                      mmcallinfo.matchnucleotidesonly,
                      &mmcallinfo.subjectfile[0]) != 0)
  {
        fprintf(stderr,"%s: %s\n",argv[0],messagespace());
        MPI::Finalize();
        return EXIT_FAILURE;
  }
  if(procmaxmatches(&mmcallinfo,&subjectmultiseq) != 0)
  {
        fprintf(stderr,"%s: %s\n",argv[0],messagespace());
        MPI::Finalize();
        return EXIT_FAILURE;
  }
  freemultiseq (&subjectmultiseq);
  checkspaceleak ();
  mmcheckspaceleak ();
  fprintf(stderr,"# COMPLETETIME %s %s %.2f\n",
         argv[0],&mmcallinfo.subjectfile[0],
         getruntime());
  fprintf(stderr,"# SPACE %s %s %.2f\n",argv[0],
         &mmcallinfo.subjectfile[0],
         MEGABYTES(getspacepeak()+mmgetspacepeak()));
        cerr << "Toci application for genome alignment under HPC environments" << endl;
        finish = MPI::Wtime();
    }
    MPI_Finalize();
    cerr << "Final Time: " << finish-start << endl;
  return EXIT_SUCCESS;
}

//}