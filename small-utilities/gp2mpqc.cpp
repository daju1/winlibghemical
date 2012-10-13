#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* This very simple program was written by Dr. Robert R. Pavlis, 
rpavlis@pittstate.edu. It may be freely distributed except it may NOT be
incorporated into any commercial program with or without permission. 
It translates ghemical output to mpqc input files that generate oogl
files for geomview.   */

// Slightly modified by TH 2000-12-13

FILE *infile,*outfile;
char filename[40],fileroot[40];

int main(int argc, char *argv[])
{
    char manstring[256], *result, element[2];
    static char elnames[]="H HeLiBeB C N O F NeNaMgAlSiP S ClArK CaScTiV CrMnFeCoNiCuZnGaGeAsSeBrKr"; 
    int i,j,n;
    double x,y,z;

if(argc>1)
 if(argv[1][0]=='-') 
 for(n=1;n<argc;n++)
   if (argv[n][0]=='-')
   if (argv[n][1]=='h')
     {
    printf("***************************************************************\n");
    printf("* This program converts ghemical mm1gp files to very generic  *\n");
    printf("* mpqc input files. They should be edited to set basis set    *\n");
    printf("* ab initio algorithms, etc.  Multiplicity should always be   *\n");
    printf("* specified on open shell molecules. Both it and charge are   *\n");
    printf("* commented out in the mpqc file produced by this program.    *\n");  
    printf("* The Input mm1gp filename must not contain more than one dot *\n");
    printf("* and it must have the file extension <.mm1gp>.               *\n");
    printf("* The output file will have the extension <.in>               *\n");
    printf("* Command line options:                                       *\n");
    printf("* -v causes mpqc to compute vibrational modes.                *\n");
    printf("* -g causes mpqc to compute seven oogl images of molecule     *\n");
    printf("* -b causes the mpqc file to have only molecule description   *\n");
    printf("* -h prints this message                                      *\n");
    printf("***************************************************************\n");
    exit(0);
     }
    if (argc==1)
        {
	printf("Enter file name without extension:\n");	
	gets(fileroot);
        }
    else
    if(argv[argc-1][0]!='-')
	{
	strcpy(fileroot, argv[argc-1]);
	}
    else
       {
	printf("Enter file name without extension:\n");
	gets(fileroot);
        }

    /*following routine is required to restart if string have full stop*/
    if ((result=strrchr(fileroot,'.'))!=NULL)
        {
	printf("Set file name must not contain <.>!\n");
        printf("Input file must have the extension <mm1gp>\n");
	exit(0);
        }
    strcpy(filename, fileroot);
    strcat(filename,".mm1gp");
    if((infile=fopen(filename,"r"))==NULL)
        {
		printf("Input file \"%s\" not opened!\n", filename);
		strcpy(filename, fileroot);
		strcat(filename,".gpr");
		if((infile=fopen(filename,"r"))==NULL)
			{
		printf("Input file \"%s\" not opened!\n", filename);
			exit(1);
			}
		//exit(1);
        }

    strcpy(filename, fileroot);
    strcat(filename,".in" );
    if((outfile=fopen(filename,"w"))==NULL)
       {
       printf("Output file could not be opened.\n");
       exit(1);
       }
fprintf(outfile,"%% Edit symmetry, guess basis set and final basis set\n");
fprintf(outfile,"molecule<Molecule>: ( \n");
fprintf(outfile,"  symmetry = C1\n");
fprintf(outfile,"  unit = angstrom\n");
fprintf(outfile,"  { atoms geometry } =\n");
fprintf(outfile,"  {\n");

// here we read the input file...
// first we search for !Atoms section, and get the number of atoms.

int atoms = -1;
int * element_table = NULL;
char buffer[128]; int tmp1; int tmp2;

while(!feof(infile))
{
	fgets(manstring,75,infile);
	if(manstring[0] != '!') continue;
	
	bool section = true;
	if (manstring[1] != 'A') section = false;
	if (manstring[2] != 't') section = false;
	if (manstring[3] != 'o') section = false;
	if (manstring[4] != 'm') section = false;
	if (manstring[5] != 's') section = false;
	if (!section) continue;
	
	sscanf(manstring,"%s %d", buffer, &atoms);
	element_table = new int[atoms];
	
	for (int loop = 0;loop < atoms;loop++)
	{
		fgets(manstring,75,infile);
		sscanf(manstring,"%d %d", &tmp1, &tmp2);
		
		element_table[loop] = tmp2;
	}
	
	break;
}

// then we look for !Coord section, and write the output.

while(!feof(infile))
{
	fgets(manstring,75,infile);
	if(manstring[0] != '!') continue;
	
	bool section = true;
	if (manstring[1] != 'C') section = false;
	if (manstring[2] != 'o') section = false;
	if (manstring[3] != 'o') section = false;
	if (manstring[4] != 'r') section = false;
	if (manstring[5] != 'd') section = false;
	if (!section) continue;
	
	for (int loop = 0;loop < atoms;loop++)
	{
		fgets(manstring,75,infile);
		sscanf(manstring,"%d %lf %lf %lf", &i, &x, &y, &z);
		
		j = element_table[loop];
		
		element[0]=elnames[2*j-2]; element[1]=elnames[2*j-1];
		fprintf(outfile,"%s [ %lf %lf %lf ] \n",element,10.0*x,10.0*y,10.0*z);
	}
	
	break;
}

delete[] element_table;

// ok...

fprintf(outfile," }\n)\n");
if(argc>2)
 for(n=1;n<argc;n++)
   if (argv[n][0]=='-')
   if (argv[n][1]=='b')
   {
   fclose(infile);
   fclose(outfile);
   exit(0);
   }
fprintf(outfile,"%% Basis set specification\n");
fprintf(outfile,"basis<GaussianBasisSet>: (\n");
fprintf(outfile,"  name =\"STO-3G\" \n");
fprintf(outfile,"  molecule = $:molecule\n");
fprintf(outfile,")\n");
fprintf(outfile,"mpqc: (\n");
fprintf(outfile,"  checkpoint = yes\n");
fprintf(outfile,"  savestate = yes\n");
fprintf(outfile,"%% Molecular coordinates for optimization\n");
fprintf(outfile,"  coor<SymmMolecularCoor>: (\n");
fprintf(outfile,"    molecule = $:molecule\n");
fprintf(outfile,"    generator<IntCoorGen>: (\n");
fprintf(outfile,"      molecule = $:molecule\n");
fprintf(outfile,"    )\n   )\n");
fprintf(outfile,"  do_energy = yes\n");
fprintf(outfile,"  do_gradient = yes\n");
fprintf(outfile,"%%  Method for computing the molecule's energy\n");
fprintf(outfile,"  mole<CLHF>: (\n");
fprintf(outfile,"    molecule = $:molecule\n");
fprintf(outfile,"    basis = $:basis\n");
fprintf(outfile,"    coor = $..:coor\n"); 
fprintf(outfile,"%%    multiplicity = 3\n");
fprintf(outfile,"%%    total_charge = 1\n");
fprintf(outfile,"    memory = 16000000\n");
fprintf(outfile,"    guess_wavefunction<CLHF>: (\n");
fprintf(outfile,"      molecule = $:molecule\n");
fprintf(outfile,"      basis<GaussianBasisSet>: (\n");
fprintf(outfile,"        molecule = $:molecule\n");
fprintf(outfile,"        name = \"STO-3G\" \n");
fprintf(outfile,"      )\n");
fprintf(outfile,"      memory = 16000000\n");
fprintf(outfile,"    )\n");
fprintf(outfile,"  )\n");
fprintf(outfile,"  optimize = yes\n");
fprintf(outfile,"%% Optimizer object for the molecular geometry\n");
fprintf(outfile,"  opt<QNewtonOpt>: (\n");
fprintf(outfile,"    max_iterations = 20\n");
fprintf(outfile,"    function = $..:mole\n");
fprintf(outfile,"    update<BFGSUpdate>: ()\n");
fprintf(outfile,"    convergence<MolEnergyConvergence>: (\n");
fprintf(outfile,"      cartesian = yes\n");
fprintf(outfile,"      energy = $..:..:mole\n");
fprintf(outfile,"    )\n   )\n");
if(argc>2)
 for(n=1;n<argc;n++)
   if (argv[n][0]=='-')
   if (argv[n][1]=='v')
     {
fprintf(outfile,"%%Vibrational frequencies\n");
fprintf(outfile,"    freq<MolecularFrequencies>: ( \n");
fprintf(outfile,"       molecule = $:molecule \n  ) \n");      
     }
fprintf(outfile,"write_pdb = yes\n");
if(argc>2)   /*if command line option g called*/
 for(n=1;n<argc;n++)
   if (argv[n][0]=='-')
   if (argv[n][1]=='g')
     {
fprintf(outfile,"  renderer<OOGLRender>:()\n");
fprintf(outfile,"  rendered: [\n");
fprintf(outfile,"    <RenderedMolecularSurface>: (\n");
fprintf(outfile,"      name = gradden\n");
fprintf(outfile,"      colorizer<GradDensityColorizer>:(\n");
fprintf(outfile,"        molecule = $:molecule\n");
fprintf(outfile,"        wfn = $:mpqc:mole\n      )\n");
fprintf(outfile,"      surface<TriangulatedImplicitSurface>: (\n");
fprintf(outfile,"        value = 0.01\n");
fprintf(outfile,"        volume<ElectronDensity>: (\n");
fprintf(outfile,"          wfn = $:mpqc:mole\n        )\n");
fprintf(outfile,"        resolution = 0.5\n");
fprintf(outfile,"        remove_short_edges = 1\n");
fprintf(outfile,"        remove_slender_triangles = 1\n");
fprintf(outfile,"        short_edge_factor = 0.4\n");
fprintf(outfile,"        slender_triangle_factor = 0.2\n      )\n");
fprintf(outfile,"      molecule = $:molecule\n");
fprintf(outfile,"      atominfo = $:atominfo\n    )\n");
fprintf(outfile,"    <RenderedMolecularSurface>: (\n");
fprintf(outfile,"      name = dencon\n");
fprintf(outfile,"      colorizer<DensityColorizer>:(\n");
fprintf(outfile,"        molecule = $:molecule\n");
fprintf(outfile,"        wfn = $:mpqc:mole\n      )\n");
fprintf(outfile,"      surface<TriangulatedImplicitSurface>: (\n");
fprintf(outfile,"        volume<ConnollyShape>: (\n");
fprintf(outfile,"          molecule = $:molecule\n");
fprintf(outfile,"          probe_radius = 2.6456173\n");
fprintf(outfile,"          atominfo = $:atominfo\n        )\n");
fprintf(outfile,"        resolution = 0.5\n");
fprintf(outfile,"        remove_short_edges = 1\n");
fprintf(outfile,"        remove_slender_triangles = 1\n");
fprintf(outfile,"        short_edge_factor = 0.4\n");
fprintf(outfile,"        slender_triangle_factor = 0.2\n      )\n");
fprintf(outfile,"      molecule = $:molecule\n");
fprintf(outfile,"      atominfo = $:atominfo\n    )\n");
fprintf(outfile,"    <RenderedStickMolecule>: (\n");
fprintf(outfile,"      name = stick\n");
fprintf(outfile,"      molecule = $:molecule\n    )\n");
fprintf(outfile,"    <RenderedBallMolecule>: (\n");
fprintf(outfile,"      name = ball\n");
fprintf(outfile,"      appearance<Appearance>: (level = 4)\n");
fprintf(outfile,"      molecule = $:molecule\n    )\n");
fprintf(outfile,"    <RenderedMolecularSurface>: (\n");
fprintf(outfile,"      name = dens\n");
fprintf(outfile,"      surface<TriangulatedImplicitSurface>: (\n");
fprintf(outfile,"        volume<ElectronDensity>: (\n");
fprintf(outfile,"          wfn = $:mpqc:mole\n        )\n");
fprintf(outfile,"        value = 0.01\n");
fprintf(outfile,"        resolution = 0.5\n");
fprintf(outfile,"        remove_short_edges = 1\n");
fprintf(outfile,"        remove_slender_triangles = 1\n");
fprintf(outfile,"        short_edge_factor = 0.4\n");
fprintf(outfile,"        slender_triangle_factor = 0.2\n      )\n");
fprintf(outfile,"      molecule = $:molecule\n");
fprintf(outfile,"      atominfo = $:atominfo\n    )\n");
fprintf(outfile,"    <RenderedMolecularSurface>: (\n");
fprintf(outfile,"      name = vdw\n");
fprintf(outfile,"      surface<TriangulatedImplicitSurface>: (\n");
fprintf(outfile,"        volume<VDWShape>: (\n");
fprintf(outfile,"           molecule = $:molecule\n");
fprintf(outfile,"           atominfo = $:atominfo\n        )\n");
fprintf(outfile,"        resolution = 0.5\n");
fprintf(outfile,"        remove_short_edges = 1\n");
fprintf(outfile,"        remove_slender_triangles = 1\n");
fprintf(outfile,"        short_edge_factor = 0.4\n");
fprintf(outfile,"        slender_triangle_factor = 0.2      )\n");
fprintf(outfile,"      molecule = $:molecule\n");
fprintf(outfile,"      atominfo = $:atominfo\n    )\n");
fprintf(outfile,"    <RenderedMolecularSurface>: (\n");
fprintf(outfile,"      name = con\n");
fprintf(outfile,"      surface<TriangulatedImplicitSurface>: (\n");
fprintf(outfile,"        volume<ConnollyShape>: (\n");
fprintf(outfile,"          molecule = $:molecule\n");
fprintf(outfile,"          probe_radius = 2.6456173\n");
fprintf(outfile,"          atominfo = $:atominfo\n        )\n");
fprintf(outfile,"        resolution = 0.5\n");
fprintf(outfile,"        remove_short_edges = 1\n");
fprintf(outfile,"        remove_slender_triangles = 1\n");
fprintf(outfile,"        short_edge_factor = 0.4\n");
fprintf(outfile,"        slender_triangle_factor = 0.2\n      )\n");
fprintf(outfile,"      molecule = $:molecule\n");
fprintf(outfile,"      atominfo = $:atominfo\n    )\n  ]");                
     }
fprintf(outfile," \n)");
fclose(infile);
fclose(outfile);
}

// end
