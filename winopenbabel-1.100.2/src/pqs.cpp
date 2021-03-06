/**********************************************************************
Copyright (C) 2003 by Pawel Wolinski

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation version 2 of the License.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

***********************************************************************/

#include "mol.h"
#include <ctype.h>

#if HAVE_STRINGS_H
#include <strings.h>
#else
#define rindex strrchr
#endif

using namespace std;

namespace OpenBabel {

/* Lower first 5 characters of each word 
 * (words separated by ' ' or '=')
 * Omit the filename after 'file=' card */
void lowerit(char *s) {  
char tmp[6];
unsigned int i, do_lower=5;
   for (i=0; i<strlen(s); i++) {
      if (s[i]==' ') do_lower=5;
      if (s[i]=='=') {
	 strncpy(tmp,&s[i-4],5); tmp[5]='\0';
	 if (strcmp(tmp,"file=")!=0) do_lower=5;
      }
      else {
         if (do_lower) {s[i]=tolower(s[i]); do_lower--;}
      }
   }
}

bool card_found(char *s) {
int i;
char *input_cards[]	={"titl","file","cpu ","text","calc", // 0-4
	                  "geom","basi","inte","gues","scf ", // 5-9
	                  "forc","intc","freq","nbo ","pop ", //10-14
		          "pop=","semi","opti","mass","nmr ", //15-19
		          "lmp2","numh","rest","nucl","mp2 ", //20-24
			  "mem=","%mem","jump","clea","stop", //25-29
			  "mtst","dyna","anfc","corr","ffld", //30-34
			  "hess","path","scan","chk=","save", //35-39
			  "scr=","thre","iter","diis","lvsh", //40-44
		          "pseu","sthr","nodd","virt","fact", //45-49
			  "gran","anne","prin","loca","dft=", //50-54
		          "cuto","preo"};		      //55-56 
   lowerit(s);
   for (i=0; i<56; i++) 
      if (strstr(s,input_cards[i])!=NULL) return true;
   return false;
}


int ReadPQS_geom(istream &ifs, OBMol &mol, const char *title, 
		 int input_style, double bohr_to_angstrom) {
int atom_count=0;
double x, y, z;
char buffer[BUFF_SIZE];
string str;
OBAtom *atom;
vector<string> vs;

   mol.Clear();
   mol.BeginModify();
 
   while (ifs.getline(buffer,BUFF_SIZE) && !card_found(buffer)) {
      if (buffer[0]!='$') {
         tokenize(vs, buffer);
         atom=mol.NewAtom();
         str=vs[0]; 
         if (input_style==0) {
            atom->SetAtomicNum(etab.GetAtomicNum(str.c_str()));
            x=atof((char*) vs[1].c_str())*bohr_to_angstrom;
            y=atof((char*) vs[2].c_str())*bohr_to_angstrom;
            z=atof((char*) vs[3].c_str())*bohr_to_angstrom;
         } else {
            str.replace (0,2,"");
            atom->SetAtomicNum(etab.GetAtomicNum(str.c_str()));
            x=atof((char*) vs[2].c_str())*bohr_to_angstrom;
            y=atof((char*) vs[3].c_str())*bohr_to_angstrom;
            z=atof((char*) vs[4].c_str())*bohr_to_angstrom;
         }
         atom->SetVector(x, y, z);
         atom_count++;
      }
   }

   mol.EndModify();
   mol.ConnectTheDots();
   mol.PerceiveBondOrders();
   mol.SetTitle(title);

   return atom_count;
}



bool ReadPQS(istream &ifs, OBMol &mol, const char *title) {
char buffer[BUFF_SIZE];
char coord_file[200];
char full_coord_path[255]="\0";
ifstream coordFileStream;
double bohr_to_angstrom=1.0;
unsigned int i, input_style, atom_count=0;
bool geom_found;

   geom_found=false;
   while (!geom_found && ifs.getline(buffer,BUFF_SIZE)) {
      lowerit(buffer);			//look for geom except in title or text
      if (strstr(buffer,"geom")!=NULL &&
	  (strncmp(buffer,"text",4)!=0 && strncmp(buffer,"titl",4)!=0)) {
	 geom_found=true;
	 lowerit(buffer);

	 if (strstr(buffer,"bohr")!=NULL) bohr_to_angstrom=0.529177249;
	 else 				  bohr_to_angstrom=1.0;
	 input_style=0;
	 if (strstr(buffer,"=tx90")!=NULL) input_style=1;
	 if (strstr(buffer,"=tx92")!=NULL) input_style=0;
	 if (strstr(buffer,"=pqs" )!=NULL) input_style=0;

	 if (strstr(buffer,"file=")!=NULL) {  //external geometry file
	    strcpy(coord_file,strstr(buffer,"file=")+5);
	    if (rindex(coord_file,' ')!=NULL) *rindex(coord_file,' ')='\0';
	    if (coord_file[0]!='/') {
	       strcpy(full_coord_path,title);
	       if (rindex(full_coord_path,'/')!=NULL) 
	          *(rindex(full_coord_path,'/')+1)='\0';
	       else strcpy(full_coord_path,"");
	    }
	    strcat(full_coord_path,coord_file);
	    cerr<<"ReadPQS: external geometry file referenced: "<< \
		    full_coord_path<<endl;

	    coordFileStream.open(full_coord_path);
	    if (!coordFileStream) {
	       cerr<<"ReadPQS: cannot read external geomentry file!"<<endl;
	       exit (-1);
	    } else {
	       ifs.seekg(0, ios::end); //move .inp file pointer to the end of file
	       if (strstr(buffer,"=car" )!=NULL) 
	          return ReadBiosymCAR(coordFileStream, mol, title);
               if (strstr(buffer,"=hin" )!=NULL) 
	          return ReadHIN(coordFileStream, mol, title);
               if (strstr(buffer,"=pdb" )!=NULL) 
	          return ReadPDB(coordFileStream, mol, title);
               if (strstr(buffer,"=mop" )!=NULL) 
	          return ReadMOPAC(coordFileStream, mol, title);
	       //probably pqs's own xyz format
	       atom_count=ReadPQS_geom(coordFileStream,mol,title,
			       input_style,bohr_to_angstrom);
	    }
	 }
      }
   }
   
   if (geom_found) {
      if (atom_count==0)        //read directly form .inp file 
         atom_count=ReadPQS_geom(ifs,mol,title,input_style,bohr_to_angstrom);
      if (atom_count==0) { 	//try .coord file
	 strcpy(coord_file,title);
	 if (rindex(coord_file,'.')!=NULL) *rindex(coord_file,'.')='\0';
	 strcat(coord_file,".coord");
	 coordFileStream.open(coord_file);
	 if (!coordFileStream) {
	    cerr<<"ReadPQS: cannot read external "<<coord_file<<" file!"<<endl;
	    exit (-1);
	 } else 
	    atom_count=ReadPQS_geom(coordFileStream,mol,title,0,
			    		bohr_to_angstrom);
      }
   } else 
      cerr<<"ReadPQS: error reading PQS file.  GEOM card not found!"<<endl;
     
   ifs.seekg(0, ios::end); //move .inp file pointer to the end of file
   if (atom_count>0) return true;
   else              return false;
}



bool WritePQS(ostream &ofs, OBMol &mol) {
unsigned int i;
char buffer[BUFF_SIZE];
OBAtom *atom;
   ofs<<"TEXT="<<mol.GetTitle()<<endl;
   ofs<<"GEOM=PQS"<<endl;
   for (i=1; i<=mol.NumAtoms(); i++) {
      atom=mol.GetAtom(i);
      sprintf(buffer,"%s           %10.6lf   %10.6lf   %10.6lf",
            etab.GetSymbol(atom->GetAtomicNum()),
            atom->GetX(),
            atom->GetY(),
            atom->GetZ());
      ofs<<buffer<<endl;
   }
   return(true);
}

} // Namespace
