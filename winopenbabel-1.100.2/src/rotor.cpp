/**********************************************************************
rotor.cpp - Rotate torsional according to rotor rules.

Copyright (C) 1998-2000 by OpenEye Scientific Software, Inc.
Some portions Copyright (c) 2001-2003 by Geoffrey R. Hutchison

This file is part of the Open Babel project.
For more information, see <http://openbabel.sourceforge.net/>

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation version 2 of the License.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.
***********************************************************************/

#include "babelconfig.h"
#include "mol.h"
#include "rotor.h"

using namespace std;
namespace OpenBabel {

#define OB_DEFAULT_DELTA 10.0
static bool GetDFFVector(OBMol&,vector<int>&,OBBitVec&);
static bool CompareRotor(const pair<OBBond*,int>&,const pair<OBBond*,int>&);


  //**************************************
  //**** OBRotorList Member Functions ****
  //**************************************

bool OBRotorList::Setup(OBMol &mol)
{
  Clear();
  FindRotors(mol);
  if (!Size()) return(false);

  SetEvalAtoms(mol);
  AssignTorVals(mol);
  
  OBRotor *rotor;
  vector<OBRotor*>::iterator i;
  for (rotor = BeginRotor(i);rotor;rotor = NextRotor(i))
    if (!rotor->Size())
      {
	ThrowError("WARNING - The following rotor has no associated torsion values");
	int ref[4];
	rotor->GetDihedralAtoms(ref);
	char buffer[BUFF_SIZE];
	sprintf(buffer,"rotor -> %d %d %d %d",ref[0],ref[1],ref[2],ref[3]);
	ThrowError(buffer);
      }

  return(true);
}

bool OBRotorList::FindRotors(OBMol &mol)
{
  mol.FindRingAtomsAndBonds();
  vector<int> gtd;
  mol.GetGTDVector(gtd);
  
  OBBond *bond;
  vector<OBEdgeBase*>::iterator i;
  vector<pair<OBBond*,int> > vtmp;

  int score;
  for (bond = mol.BeginBond(i);bond;bond = mol.NextBond(i))
    if (bond->IsRotor())
      {
		if (HasFixedAtoms() && IsFixedBond(bond)) continue;
		score = gtd[bond->GetBeginAtomIdx()-1] + 
			gtd[bond->GetEndAtomIdx()-1];
		vtmp.push_back(pair<OBBond*,int> (bond,score));
      }

  sort(vtmp.begin(),vtmp.end(),CompareRotor);

  OBRotor *rotor;int count;
  vector<pair<OBBond*,int> >::iterator j;
  for (j = vtmp.begin(),count=0;j != vtmp.end();j++,count++)
    {
      rotor = new OBRotor;
      rotor->SetBond((*j).first);
      rotor->SetIdx(count);
      rotor->SetNumCoords(mol.NumAtoms()*3);
      _rotor.push_back(rotor);
    }

  return(true);
}

bool OBRotorList::IsFixedBond(OBBond *bond)
{
  OBAtom *a1,*a2,*a3;
  vector<OBEdgeBase*>::iterator i;

  a1 = bond->GetBeginAtom();
  a2 = bond->GetEndAtom();
  if (!_fix[a1->GetIdx()] || !_fix[a2->GetIdx()]) return(false);

  bool isfixed=false;
  for (a3 = a1->BeginNbrAtom(i);a3;a3 = a1->NextNbrAtom(i))
    if (a3 != a2 && _fix[a3->GetIdx()])
      {isfixed = true;break;}

  if (!isfixed) return(false);

  isfixed = false;
  for (a3 = a2->BeginNbrAtom(i);a3;a3 = a2->NextNbrAtom(i))
    if (a3 != a1 && _fix[a3->GetIdx()])
      {isfixed = true;break;}
  
  return(isfixed);
}

bool GetDFFVector(OBMol &mol,vector<int> &dffv,OBBitVec &bv)
{
  dffv.clear();
  dffv.resize(mol.NumAtoms());
  
  int dffcount,natom;
  OBBitVec used,curr,next;
  OBAtom *atom,*atom1;
  OBBond *bond;
  vector<OBNodeBase*>::iterator i;
  vector<OBEdgeBase*>::iterator j;

  next.Clear();

  for (atom = mol.BeginAtom(i);atom;atom = mol.NextAtom(i))
    {
      if (bv[atom->GetIdx()])
	{
	  dffv[atom->GetIdx()-1] = 0;
	  continue;
	}

      dffcount = 0;
      used.Clear();curr.Clear();
      used.SetBitOn(atom->GetIdx());
      curr.SetBitOn(atom->GetIdx());

      while (!curr.IsEmpty() && (bv&curr).Empty())
	{
	  next.Clear();
	  for (natom = curr.NextBit(-1);natom != curr.EndBit();natom = curr.NextBit(natom))
	    {
	      atom1 = mol.GetAtom(natom);
	      for (bond = atom1->BeginBond(j);bond;bond = atom1->NextBond(j))
	if (!used.BitIsOn(bond->GetNbrAtomIdx(atom1)) &&
		    !curr.BitIsOn(bond->GetNbrAtomIdx(atom1)))
		  if (!(bond->GetNbrAtom(atom1))->IsHydrogen())
		    next.SetBitOn(bond->GetNbrAtomIdx(atom1));
	    }

	  used |= next;
	  curr = next;
	  dffcount++;
	}

      dffv[atom->GetIdx()-1] = dffcount;
    }

  return(true);
}


static double MinimumPairRMS(OBMol&,double*,double*,bool &);

//! Rotates each bond to zero and 180 degrees and tests
//! if the 2 conformers are duplicates.  if so - the symmetric torsion
//! values are removed from consideration during a search
void OBRotorList::RemoveSymVals(OBMol &mol)
{
  double *c,*c1,*c2;
  c1 = new double [mol.NumAtoms()*3];
  c2 = new double [mol.NumAtoms()*3];
  c = mol.GetCoordinates();
  bool one2one;
  double cutoff = 0.20;

  OBRotor *rotor;
  vector<OBRotor*>::iterator i;
  for (rotor = BeginRotor(i);rotor;rotor = NextRotor(i))
    {
      //look for 2-fold symmetry about a bond
      memcpy(c1,c,sizeof(double)*mol.NumAtoms()*3);
      memcpy(c2,c,sizeof(double)*mol.NumAtoms()*3);
      rotor->SetToAngle(c1,(double)(0.0*DEG_TO_RAD));
      rotor->SetToAngle(c2,(double)(180.0*DEG_TO_RAD));

      if (MinimumPairRMS(mol,c1,c2,one2one) <cutoff && !one2one)
	{
	  rotor->RemoveSymTorsionValues(2);
	  OBBond *bond = rotor->GetBond();
	  
	  if (!_quiet)
	    {
	      cout << "symmetry found = " << ' ';
	      cout << bond->GetBeginAtomIdx() << ' ' << bond->GetEndAtomIdx() << ' ' ;
	      cout << "rms = " << ' ';
	      cout << MinimumPairRMS(mol,c1,c2,one2one) << endl;
	    }
	  continue;
	}

      //look for 3-fold symmetry about a bond
      memcpy(c1,c,sizeof(double)*mol.NumAtoms()*3);
      memcpy(c2,c,sizeof(double)*mol.NumAtoms()*3);
      rotor->SetToAngle(c1,(double)(0.0*DEG_TO_RAD)); 
      rotor->SetToAngle(c2,(double)(120.0*DEG_TO_RAD));

      if (MinimumPairRMS(mol,c1,c2,one2one) <cutoff && !one2one)
	{
	  rotor->RemoveSymTorsionValues(3);
	  OBBond *bond = rotor->GetBond();
	  
	  if (!_quiet)
	    {
	      cout << "3-fold symmetry found = " << ' ';
	      cout << bond->GetBeginAtomIdx() << ' ' << bond->GetEndAtomIdx() << ' ' ;
	      cout << "rms = " << ' ';
	      cout << MinimumPairRMS(mol,c1,c2,one2one) << endl;
	    }
	}
    }

  delete [] c1;
  delete [] c2;

  //pattern based duplicate removal
  int ref[4];
  vector<vector<int> > mlist;
  vector<vector<int> >::iterator k;
  vector<pair<OBSmartsPattern*,pair<int,int> > >::iterator j;
  for (j = _vsym2.begin();j != _vsym2.end();j++)
    if (j->first->Match(mol))
      {
	mlist = j->first->GetUMapList();

	for (k = mlist.begin();k != mlist.end();k++)
	  for (rotor = BeginRotor(i);rotor;rotor = NextRotor(i))
	    {
	      rotor->GetDihedralAtoms(ref);
	      if (((*k)[j->second.first] == ref[1] && (*k)[j->second.second] == ref[2]) ||
		  ((*k)[j->second.first] == ref[2] && (*k)[j->second.second] == ref[1]))
		{
		  rotor->RemoveSymTorsionValues(2);
		  if (!_quiet)
		    {
		      cout << "2-fold pattern-based symmetry found = " << ' ';
		      cout << ref[1] << ' ' << ref[2] << endl;
		    }
		}
	    }
      }

  for (j = _vsym3.begin();j != _vsym3.end();j++)
    if (j->first->Match(mol))
      {
	mlist = j->first->GetUMapList();

	for (k = mlist.begin();k != mlist.end();k++)
	  for (rotor = BeginRotor(i);rotor;rotor = NextRotor(i))
	    {
	      rotor->GetDihedralAtoms(ref);
	      if (((*k)[j->second.first] == ref[1] && (*k)[j->second.second] == ref[2]) ||
		  ((*k)[j->second.first] == ref[2] && (*k)[j->second.second] == ref[1]))
		{
		  rotor->RemoveSymTorsionValues(3);
		  if (!_quiet)
		    {
		      cout << "3-fold pattern-based symmetry found = " << ' ';
		      cout << ref[1] << ' ' << ref[2] << endl;
		    }
		}
	    }
      }
}

static double MinimumPairRMS(OBMol &mol,double *a,double *b,bool &one2one)
{
  int i,j,k=0;
  double min,tmp,d_2 = 0.0;
  OBBitVec bset;
  one2one = true;
  vector<OBNodeBase*> _atom;
  _atom.resize(mol.NumAtoms());
  for (i = 0;i < (signed)mol.NumAtoms();i++) _atom[i] = mol.GetAtom(i+1);

  for (i = 0;i < (signed)mol.NumAtoms();i++)
    {
      min = 10E10;
      for (j = 0;j < (signed)mol.NumAtoms();j++)
	if ((_atom[i])->GetAtomicNum() == (_atom[j])->GetAtomicNum() &&
	    (_atom[i])->GetHyb()       == (_atom[j])->GetHyb())
	  if (!bset[j])
	    {
	      tmp = SQUARE(a[3*i]-b[3*j]) + 
		    SQUARE(a[3*i+1]-b[3*j+1]) + 
		    SQUARE(a[3*i+2]-b[3*j+2]);
	      if (tmp < min)
		{
		  k = j;
		  min = tmp;
		}
	    }

      if (i != j) one2one = false;
      bset.SetBitOn(k);
      d_2 += min;
    }

  d_2 /= (double)mol.NumAtoms();
  
  return(sqrt(d_2));
}

//! Determines which atoms the internal energy should be calculated
//! if a the dihedral angle of the rotor is modified
bool OBRotorList::SetEvalAtoms(OBMol &mol)
{
  int j;
  OBBond *bond;
  OBAtom *a1,*a2;
  OBRotor *rotor;
  vector<OBRotor*>::iterator i;
  OBBitVec eval,curr,next;
  vector<OBEdgeBase*>::iterator k;

  for (rotor = BeginRotor(i);rotor;rotor = NextRotor(i))
    {
      bond = rotor->GetBond();
      curr.Clear();eval.Clear();
      curr.SetBitOn(bond->GetBeginAtomIdx());
      curr.SetBitOn(bond->GetEndAtomIdx());
      eval |= curr;

      //follow all non-rotor bonds and add atoms to eval list
      for (;!curr.Empty();)
	{
	  next.Clear();
	  for (j = curr.NextBit(0);j != curr.EndBit();j = curr.NextBit(j))
	    {
	      a1 = mol.GetAtom(j);
	      for (a2 = a1->BeginNbrAtom(k);a2;a2 = a1->NextNbrAtom(k))
		if (!eval[a2->GetIdx()])
		  if (!((OBBond*)*k)->IsRotor()||(HasFixedAtoms()&&IsFixedBond((OBBond*)*k)))
		    {
		      next.SetBitOn(a2->GetIdx());
		      eval.SetBitOn(a2->GetIdx());
		    }
	    }
	  curr = next;
	}

      //add atoms alpha to eval list
      next.Clear();
      for (j = eval.NextBit(0);j != eval.EndBit();j = eval.NextBit(j))
	{
	  a1 = mol.GetAtom(j);
	  for (a2 = a1->BeginNbrAtom(k);a2;a2 = a1->NextNbrAtom(k))
	    next.SetBitOn(a2->GetIdx());
	}
      eval |= next;
      rotor->SetEvalAtoms(eval);
    }

  return(true);
}

bool OBRotorList::AssignTorVals(OBMol &mol)
{
  OBBond *bond;
  OBRotor *rotor;
  vector<OBRotor*>::iterator i;

  int ref[4];
  double delta;
  vector<double> res;
  vector<int> itmp1;
  vector<int>::iterator j;
  for (i = _rotor.begin();i != _rotor.end();i++)
    {
      rotor=*i;
      bond = rotor->GetBond();
      _rr.GetRotorIncrements(mol,bond,ref,res,delta);
      rotor->SetTorsionValues(res);
      rotor->SetDelta(delta);

      mol.FindChildren(itmp1,ref[1],ref[2]);
      if (itmp1.size()+1 > mol.NumAtoms()/2)
	{
	  itmp1.clear();
	  mol.FindChildren(itmp1,ref[2],ref[1]);
	  swap(ref[0],ref[3]);swap(ref[1],ref[2]);
	}

      for (j = itmp1.begin();j != itmp1.end();j++) *j = ((*j)-1)*3;
      rotor->SetRotAtoms(itmp1);
      rotor->SetDihedralAtoms(ref);
    }

  return(true);
}

bool OBRotorList::SetRotAtoms(OBMol &mol)
{
  OBRotor *rotor;
  vector<int> rotatoms,dihed;
  vector<OBRotor*>::iterator i;

  int ref[4];
  vector<int>::iterator j;
  for (rotor = BeginRotor(i);rotor;rotor = NextRotor(i))
    {
      dihed = rotor->GetDihedralAtoms();
      ref[0] = dihed[0]/3+1;      ref[1] = dihed[1]/3+1;
      ref[2] = dihed[2]/3+1;      ref[3] = dihed[3]/3+1;

      mol.FindChildren(rotatoms,ref[1],ref[2]);
      if (rotatoms.size()+1 > mol.NumAtoms()/2)
	{
	  rotatoms.clear();
	  mol.FindChildren(rotatoms,ref[2],ref[1]);
	  swap(ref[0],ref[3]);swap(ref[1],ref[2]);
	}

      for (j = rotatoms.begin();j != rotatoms.end();j++) *j = ((*j)-1)*3;
      rotor->SetRotAtoms(rotatoms);
      rotor->SetDihedralAtoms(ref);
    }

  return(true);
}

void OBRotorList::SetRotAtomsByFix(OBMol &mol)
{
  int ref[4];
  OBRotor *rotor;
  vector<int> rotatoms,dihed;
  vector<int>::iterator j;
  vector<OBRotor*>::iterator i;

  GetDFFVector(mol,_dffv,_fix);

  for (rotor = BeginRotor(i);rotor;rotor = NextRotor(i))
    {
      rotatoms.clear();
      dihed = rotor->GetDihedralAtoms();
      ref[0] = dihed[0]/3+1;      ref[1] = dihed[1]/3+1;
      ref[2] = dihed[2]/3+1;      ref[3] = dihed[3]/3+1;

      if (_fix[ref[1]] && _fix[ref[2]])
	{
	  if (!_fix[ref[0]])
	    {
	      swap(ref[0],ref[3]);swap(ref[1],ref[2]);
	      mol.FindChildren(rotatoms,ref[1],ref[2]);
	      for (j = rotatoms.begin();j != rotatoms.end();j++) 
		*j = ((*j)-1)*3;
	      rotor->SetRotAtoms(rotatoms);
	      rotor->SetDihedralAtoms(ref);
	    }
	}
      else
	if (_dffv[ref[1]-1] > _dffv[ref[2]-1])
	  {
	    swap(ref[0],ref[3]);swap(ref[1],ref[2]);
	    mol.FindChildren(rotatoms,ref[1],ref[2]);
	    for (j = rotatoms.begin();j != rotatoms.end();j++) *j = ((*j)-1)*3;
	    rotor->SetRotAtoms(rotatoms);
	    rotor->SetDihedralAtoms(ref);
	  }
    }
}

OBRotorList::OBRotorList() 
{
  _rotor.clear();
  _quiet=false;
  _removesym=true;

  //para-disub benzene
  OBSmartsPattern *sp;
  sp = new OBSmartsPattern;
  sp->Init("*c1[cD2][cD2]c(*)[cD2][cD2]1");
  _vsym2.push_back(pair<OBSmartsPattern*,pair<int,int> > (sp,pair<int,int> (0,1)));

  //piperidine amide
  sp = new OBSmartsPattern;
  sp->Init("O=CN1[CD2][CD2][CD2][CD2][CD2]1");
  _vsym2.push_back(pair<OBSmartsPattern*,pair<int,int> > (sp,pair<int,int> (1,2)));

  //terminal phosphate
  sp = new OBSmartsPattern;
  sp->Init("[#8D2][#15,#16](~[#8D1])(~[#8D1])~[#8D1]");
  _vsym3.push_back(pair<OBSmartsPattern*,pair<int,int> > (sp,pair<int,int> (0,1)));

}

OBRotorList::~OBRotorList()
{
  vector<OBRotor*>::iterator i;
  for (i = _rotor.begin();i != _rotor.end();i++)  delete *i;

  vector<pair<OBSmartsPattern*,pair<int,int> > >::iterator j;
  for (j = _vsym2.begin();j != _vsym2.end();j++)
    delete j->first;

  for (j = _vsym3.begin();j != _vsym3.end();j++)
    delete j->first;
}

void OBRotorList::Clear()
{
  vector<OBRotor*>::iterator i;
  for (i = _rotor.begin();i != _rotor.end();i++)  delete *i;
  _rotor.clear();
  _fix.Clear();
}

bool CompareRotor(const pair<OBBond*,int> &a,const pair<OBBond*,int> &b)
{
  //return(a.second > b.second); //outside->in
  return(a.second < b.second);   //inside->out
}

//**********************************
//**** OBRotor Member Functions ****
//**********************************

OBRotor::OBRotor()
{
  _delta = 10.0;
  _rotatoms = NULL;
}

double OBRotor::CalcTorsion(double *c)
{
  double v1x,v1y,v1z,v2x,v2y,v2z,v3x,v3y,v3z;
  double c1x,c1y,c1z,c2x,c2y,c2z,c3x,c3y,c3z;
  double c1mag,c2mag,ang,costheta;

  //
  //calculate the torsion angle
  //
  v1x = c[_torsion[0]] -  c[_torsion[1]];
  v1y = c[_torsion[0]+1] - c[_torsion[1]+1]; 
  v1z = c[_torsion[0]+2] - c[_torsion[1]+2]; 
  v2x = c[_torsion[1]] - c[_torsion[2]];
  v2y = c[_torsion[1]+1] - c[_torsion[2]+1];
  v2z = c[_torsion[1]+2] - c[_torsion[2]+2];
  v3x = c[_torsion[2]]   - c[_torsion[3]];
  v3y = c[_torsion[2]+1] - c[_torsion[3]+1];
  v3z = c[_torsion[2]+2] - c[_torsion[3]+2];

  c1x = v1y*v2z - v1z*v2y;   c2x = v2y*v3z - v2z*v3y;
  c1y = -v1x*v2z + v1z*v2x;  c2y = -v2x*v3z + v2z*v3x;
  c1z = v1x*v2y - v1y*v2x;   c2z = v2x*v3y - v2y*v3x;
  c3x = c1y*c2z - c1z*c2y;
  c3y = -c1x*c2z + c1z*c2x;
  c3z = c1x*c2y - c1y*c2x; 
  
  c1mag = SQUARE(c1x)+SQUARE(c1y)+SQUARE(c1z);
  c2mag = SQUARE(c2x)+SQUARE(c2y)+SQUARE(c2z);
  if (c1mag*c2mag < 0.01) costheta = 1.0; //avoid div by zero error
  else costheta = (c1x*c2x + c1y*c2y + c1z*c2z)/(sqrt(c1mag*c2mag));

  if (costheta < -0.9999999) costheta = -0.9999999;
  if (costheta >  0.9999999) costheta =  0.9999999;
			      
  if ((v2x*c3x + v2y*c3y + v2z*c3z) > 0.0) ang = -acos(costheta);
  else                                     ang = acos(costheta);

  return(ang);
}

double OBRotor::CalcBondLength(double *c)
{
  double dx,dy,dz;

  dx = c[_torsion[1]] - c[_torsion[2]];
  dy = c[_torsion[1]+1] - c[_torsion[2]+1];
  dz = c[_torsion[1]+2] - c[_torsion[2]+2];
  return(sqrt(SQUARE(dx)+SQUARE(dy)+SQUARE(dz)));
}

void OBRotor::Precalc(vector<double*> &cv)
{
  double *c,ang;
  vector<double*>::iterator i;
  vector<double>::iterator j;
  vector<double> cs,sn,t;
  for (i = cv.begin();i != cv.end();i++)
    {
      c = *i;
      cs.clear();sn.clear();t.clear();
      ang = CalcTorsion(c);

      for (j = _res.begin();j != _res.end();j++)
	{
		  double cs_ = cos(*j-ang);
	  cs.push_back(cs_);
	  double sn_ = sin(*j-ang);
	  sn.push_back(sn_);
	  t.push_back(1 - cos(*j-ang));
	}

      _cs.push_back(cs);
      _sn.push_back(sn);
      _t.push_back(t);
      _invmag.push_back(1.0/CalcBondLength(c));
    }
}


void OBRotor::SetRotor(double *c,int idx,int prev)
{
  double ang,sn,cs,t,dx,dy,dz,mag;
  
  if (prev == -1) ang = _res[idx] - CalcTorsion(c);
  else            ang = _res[idx] - _res[prev];

  sn = sin(ang); cs = cos(ang);t = 1 - cs;

  dx = c[_torsion[1]]   - c[_torsion[2]];
  dy = c[_torsion[1]+1] - c[_torsion[2]+1];
  dz = c[_torsion[1]+2] - c[_torsion[2]+2];
  mag = sqrt(SQUARE(dx) + SQUARE(dy) + SQUARE(dz));

  Set(c,sn,cs,t,1.0/mag);
}

void OBRotor::Precompute(double *c)
{
  double dx,dy,dz;
  dx = c[_torsion[1]]   - c[_torsion[2]];
  dy = c[_torsion[1]+1] - c[_torsion[2]+1];
  dz = c[_torsion[1]+2] - c[_torsion[2]+2];
  _imag = 1.0/sqrt(SQUARE(dx) + SQUARE(dy) + SQUARE(dz));

  _refang = CalcTorsion(c);
}

void OBRotor::Set(double *c,int idx)
{
  double ang,sn,cs,t;

  ang = _res[idx] - _refang;
  sn = sin(ang); cs = cos(ang); t = 1-cs;
  
  double x,y,z,tx,ty,tz,m[9];

  x = c[_torsion[1]]   - c[_torsion[2]];
  y = c[_torsion[1]+1] - c[_torsion[2]+1];
  z = c[_torsion[1]+2] - c[_torsion[2]+2];

  x *= _imag; y *= _imag; z *= _imag; //normalize the rotation vector

  //set up the rotation matrix
  tx = t*x;ty = t*y;tz = t*z;
  m[0]= tx*x + cs;     m[1] = tx*y + sn*z;  m[2] = tx*z - sn*y;
  m[3] = tx*y - sn*z;  m[4] = ty*y + cs;    m[5] = ty*z + sn*x;
  m[6] = tx*z + sn*y;  m[7] = ty*z - sn*x;  m[8] = tz*z + cs;

  //
  //now the matrix is set - time to rotate the atoms
  //
  tx = c[_torsion[1]];ty = c[_torsion[1]+1];tz = c[_torsion[1]+2];
  int i,j;
  for (i = 0;i < _size;i++)
    {
      j = _rotatoms[i];
      c[j] -= tx;c[j+1] -= ty;c[j+2]-= tz;
      x = c[j]*m[0] + c[j+1]*m[1] + c[j+2]*m[2];
      y = c[j]*m[3] + c[j+1]*m[4] + c[j+2]*m[5];
      z = c[j]*m[6] + c[j+1]*m[7] + c[j+2]*m[8];
      c[j] = x+tx; c[j+1] = y+ty; c[j+2] = z+tz;
    }
}

void OBRotor::Set(double *c,double sn,double cs,double t,double invmag)
{
  double x,y,z,tx,ty,tz,m[9];

  x = c[_torsion[1]]   - c[_torsion[2]];
  y = c[_torsion[1]+1] - c[_torsion[2]+1];
  z = c[_torsion[1]+2] - c[_torsion[2]+2];

  //normalize the rotation vector

  x *= invmag; y *= invmag; z *= invmag;

  //set up the rotation matrix
  tx = t*x;ty = t*y;tz = t*z;
  m[0]= tx*x + cs;     m[1] = tx*y + sn*z;  m[2] = tx*z - sn*y;
  m[3] = tx*y - sn*z;  m[4] = ty*y + cs;    m[5] = ty*z + sn*x;
  m[6] = tx*z + sn*y;  m[7] = ty*z - sn*x;  m[8] = tz*z + cs;

  //
  //now the matrix is set - time to rotate the atoms
  //
  tx = c[_torsion[1]];ty = c[_torsion[1]+1];tz = c[_torsion[1]+2];
  int i,j;
  for (i = 0;i < _size;i++)
    {
      j = _rotatoms[i];
      c[j] -= tx;c[j+1] -= ty;c[j+2]-= tz;
      x = c[j]*m[0] + c[j+1]*m[1] + c[j+2]*m[2];
      y = c[j]*m[3] + c[j+1]*m[4] + c[j+2]*m[5];
      z = c[j]*m[6] + c[j+1]*m[7] + c[j+2]*m[8];
      c[j] = x+tx; c[j+1] = y+ty; c[j+2] = z+tz;
    }
}

void OBRotor::RemoveSymTorsionValues(int fold)
{
  vector<double>::iterator i;
  vector<double> tv;
  if (_res.size() == 1) return;

  for (i = _res.begin();i != _res.end();i++)
    if (*i >= 0.0)
      {
	if (fold == 2 && *i < DEG_TO_RAD*180.0) tv.push_back(*i);
	if (fold == 3 && *i < DEG_TO_RAD*120.0) tv.push_back(*i);
      }

  if (tv.empty()) return;
  _res = tv;
}

void OBRotor::SetDihedralAtoms(int ref[4])
{
  for (int i = 0;i < 4;i++) _ref[i] = ref[i];
  _torsion.resize(4);
  _torsion[0] = (ref[0]-1)*3; _torsion[1] = (ref[1]-1)*3;
  _torsion[2] = (ref[2]-1)*3; _torsion[3] = (ref[3]-1)*3;
}

void OBRotor::SetRotAtoms(vector<int> &vi) 
{
  if (_rotatoms) delete [] _rotatoms;
  _rotatoms = new int [vi.size()];
  copy(vi.begin(),vi.end(),_rotatoms);
  _size = vi.size();
}

//***************************************
//**** OBRotorRules Member functions ****
//***************************************

static char TorsionDefaults[] = 
{0x53,0x50,0x33,0x2D,0x53,0x50,0x33,0x20,0x20,0x20,0x36,0x30,0x2E,0x30,0x20
,0x2D,0x36,0x30,0x2E,0x30,0x20,0x31,0x38,0x30,0x2E,0x30,0x20,0x0A,0x53,0x50
,0x32,0x2D,0x53,0x50,0x32,0x20,0x20,0x20,0x30,0x2E,0x30,0x20,0x31,0x38,0x30
,0x2E,0x30,0x20,0x2D,0x33,0x30,0x2E,0x30,0x20,0x33,0x30,0x2E,0x30,0x20,0x31
,0x35,0x30,0x2E,0x30,0x20,0x2D,0x31,0x35,0x30,0x2E,0x30,0x20,0x0A,0x53,0x50
,0x33,0x2D,0x53,0x50,0x32,0x20,0x20,0x20,0x30,0x2E,0x30,0x20,0x33,0x30,0x2E
,0x30,0x20,0x2D,0x33,0x30,0x2E,0x30,0x20,0x36,0x30,0x2E,0x30,0x20,0x2D,0x36
,0x30,0x2E,0x30,0x20,0x31,0x32,0x30,0x2E,0x30,0x20,0x2D,0x31,0x32,0x30,0x2E
,0x30,0x20,0x2D,0x31,0x35,0x30,0x2E,0x30,0x20,0x31,0x35,0x30,0x2E,0x30,0x20
,0x31,0x38,0x30,0x2E,0x30,0x20,0x2D,0x39,0x30,0x2E,0x30,0x20,0x39,0x30,0x2E
,0x30,0x0A}; 

OBRotorRules::OBRotorRules()
{
  _quiet=false;
  _init = false;
  _dir = BABEL_DATADIR;
  _envvar = "BABEL_DATADIR";
  _filename = "torlib.txt";
  _subdir = "omega";
  _dataptr = TorsionDefaults;
}

void OBRotorRules::ParseLine(const char *buffer)
{
  int i;
  int ref[4];
  double delta;
  vector<double> vals;
  vector<string> vs;
  vector<string>::iterator j;
  char temp_buffer[BUFF_SIZE];

  if (buffer[0] == '#') return;
  tokenize(vs,buffer);
  if (vs.empty()) return;

  if (EQn(buffer,"SP3-SP3",7))
    {
      _sp3sp3.clear();
      for (j = vs.begin(),j++;j != vs.end();j++)
	_sp3sp3.push_back(DEG_TO_RAD*atof(j->c_str()));
      return;
    }

  if (EQn(buffer,"SP3-SP2",7))
    {
      _sp3sp2.clear();
      for (j = vs.begin(),j++;j != vs.end();j++)
	_sp3sp2.push_back(DEG_TO_RAD*atof(j->c_str()));
      return;
    }

  if (EQn(buffer,"SP2-SP2",7))
    {
      _sp2sp2.clear();
      for (j = vs.begin(),j++;j != vs.end();j++)
	_sp2sp2.push_back(DEG_TO_RAD*atof(j->c_str()));
      return;
    }

  if (!vs.empty() && vs.size() > 5)
    {
      strcpy(temp_buffer,vs[0].c_str());
      //reference atoms
      for (i = 0;i < 4;i++) ref[i] = atoi(vs[i+1].c_str())-1;
      //possible torsions
      vals.clear();
      delta = OB_DEFAULT_DELTA;
      for (i = 5;(unsigned)i < vs.size();i++) 
	{
	  if (i == (signed)(vs.size()-2) && vs[i] == "Delta")
	    {
	      delta = atof(vs[i+1].c_str());
	      i += 2;
	    }
	  else vals.push_back(DEG_TO_RAD*atof(vs[i].c_str()));
	}

      if (vals.empty())
	{
	  ThrowError("DANGER WILL ROBINSON!!!");
	  string err = "The following rule has no associated torsions: ";
	  err += vs[0];
	  ThrowError(err);
	}
      OBRotorRule *rr = new OBRotorRule (temp_buffer,ref,vals,delta);
      if (rr->IsValid())
	_vr.push_back(rr);
      else
	delete rr;
    }

}

void OBRotorRules::GetRotorIncrements(OBMol &mol,OBBond *bond,
				      int ref[4],vector<double> &vals,double &delta)
{
  vals.clear();
  vector<pair<int,int> > vpr;
  vpr.push_back(pair<int,int> (0,bond->GetBeginAtomIdx()));
  vpr.push_back(pair<int,int> (0,bond->GetEndAtomIdx()));

  delta = OB_DEFAULT_DELTA;

  int j;
  OBSmartsPattern *sp;
  vector<vector<int> > map;
  vector<OBRotorRule*>::iterator i;
  for (i = _vr.begin();i != _vr.end();i++)
    {
      sp = (*i)->GetSmartsPattern();
      (*i)->GetReferenceAtoms(ref);
      vpr[0].first = ref[1]; vpr[1].first = ref[2];

      if (!sp->RestrictedMatch(mol,vpr,true))
	  {
		  swap(vpr[0].first,vpr[1].first);
		  if (!sp->RestrictedMatch(mol,vpr,true))
			  continue;
	  }

	  map = sp->GetMapList();
	  for (j = 0;j < 4;j++) ref[j] = map[0][ref[j]];
	  vals = (*i)->GetTorsionVals();
	  delta = (*i)->GetDelta();

	  OBAtom *a1,*a2,*a3,*a4,*r;
	  a1 = mol.GetAtom(ref[0]); a4 = mol.GetAtom(ref[3]);
	  if (a1->IsHydrogen() && a4->IsHydrogen()) continue; //don't allow hydrogens at both ends
	  if (a1->IsHydrogen() || a4->IsHydrogen()) //need a heavy atom reference - can use hydrogen
	  {
		  bool swapped = false;
			a2 = mol.GetAtom(ref[1]); a3 = mol.GetAtom(ref[2]);
			if (a4->IsHydrogen()) {swap(a1,a4); swap(a2,a3); swapped = true;} 

			vector<OBEdgeBase*>::iterator k;
			for (r = a2->BeginNbrAtom(k);r;r = a2->NextNbrAtom(k))
				if (!r->IsHydrogen() && r != a3)
					break;

			if (!r) continue; //unable to find reference heavy atom
//			cerr << "r = " << r->GetIdx() << endl;

			double t1 = mol.GetTorsion(a1,a2,a3,a4);
			double t2 = mol.GetTorsion(r,a2,a3,a4);
			double diff = t2 - t1;
			if (diff > 180.0) diff -= 360.0;
			if (diff < -180.0) diff += 360.0;
			diff *= DEG_TO_RAD;

			vector<double>::iterator m;
			for (m = vals.begin();m != vals.end();m++) 
			{
				*m += diff;
				if (*m < PI) *m += 2.0*PI;
				if (*m > PI) *m -= 2.0*PI;
			}

			if (swapped) ref[3] = r->GetIdx();
			else         ref[0] = r->GetIdx();

/*
	  mol.SetTorsion(r,a2,a3,a4,vals[0]);
	  cout << "test = " << (vals[0]-diff)*RAD_TO_DEG << ' ';
	  cout << mol.GetTorsion(a1,a2,a3,a4) <<  ' ';
	  cout << mol.GetTorsion(r,a2,a3,a4) << endl;
	  */
	  }

	  char buffer[BUFF_SIZE];
	  if (!_quiet)
	  {
		  sprintf(buffer,"%3d%3d%3d%3d %s",
			  ref[0],ref[1],ref[2],ref[3],
			  ((*i)->GetSmartsString()).c_str());
		  cout << buffer << endl;
	  }
	  return;
    }

  //***didn't match any rules - assign based on hybridization***
  OBAtom *a1,*a2,*a3,*a4;
  a2 = bond->GetBeginAtom(); a3 = bond->GetEndAtom();
  vector<OBEdgeBase*>::iterator k;

  for (a1 = a2->BeginNbrAtom(k);a1;a1 = a2->NextNbrAtom(k))
	  if (!a1->IsHydrogen() && a1 != a3)
		  break;
  for (a4 = a3->BeginNbrAtom(k);a4;a4 = a3->NextNbrAtom(k))
	  if (!a4->IsHydrogen() && a4 != a2)
		  break;

  ref[0] = a1->GetIdx(); ref[1] = a2->GetIdx();
  ref[2] = a3->GetIdx(); ref[3] = a4->GetIdx();

  if (a2->GetHyb() == 3 && a3->GetHyb() == 3) //sp3-sp3
    {
      vals = _sp3sp3;

      if (!_quiet)
	{
	  char buffer[BUFF_SIZE];
	  sprintf(buffer,"%3d%3d%3d%3d %s",
		  ref[0],ref[1],ref[2],ref[3],"sp3-sp3");
	  cout << buffer << endl;
	}
    }
  else
  if (a2->GetHyb() == 2 && a3->GetHyb() == 2) //sp2-sp2
    {
      vals = _sp2sp2;

      if (!_quiet)
	{
	  char buffer[BUFF_SIZE];
	  sprintf(buffer,"%3d%3d%3d%3d %s",
		  ref[0],ref[1],ref[2],ref[3],"sp2-sp2");
	  cout << buffer << endl;
	}
    }
  else //must be sp2-sp3
    {
      vals = _sp3sp2;

      if (!_quiet)
	{
	  char buffer[BUFF_SIZE];
	  sprintf(buffer,"%3d%3d%3d%3d %s",
		  ref[0],ref[1],ref[2],ref[3],"sp2-sp3");
	  cout << buffer << endl;
	}
    }
}

OBRotorRules::~OBRotorRules()
{
  vector<OBRotorRule*>::iterator i;
  for (i = _vr.begin();i != _vr.end();i++)
    delete (*i);
}

#undef OB_DEFAULT_DELTA	  
}

