// ENG1_SF.H : calculations for simplified protein models (molecular mechanics).

// Copyright (C) 1998 Tommi Hassinen.

// This program is free software; you can redistribute it and/or modify it
// under the terms of the license (GNU GPL) which comes with this package.

/*################################################################################################*/

#ifndef ENG1_SF_H
#define ENG1_SF_H

#include "libconfig.h"

struct eng1_sf_param;

class sf_chn;		// chain
class sf_res;		// residue

class sf_dsb;		// disulphide bridge

class sf_helix4info;
class sf_strandinfo;
class sf_strandpair;

class setup1_sf;

struct sf_bt1_data;		// saved distance results.
struct sf_bt2_data;		// saved angle results.
struct sf_nonbonded_lookup;

struct sf_bt1;			// virtual-bond stretching.
struct sf_bt2;			// virtual-angle bending.
struct sf_bt3;			// main-chain torsion/dipole terms.
struct sf_bt4;			// 1st side-chain virtual-atom terms.

struct sf_nbt1;			// nonbonded terms.

struct sf_nbt3_nd;
struct sf_nbt3_ipd;

struct sf_nbt3_nl;
struct sf_nbt3_coi;
struct sf_nbt3_ips;
struct sf_nbt3_arc;

// NOT YET PROPERLY TESTED!!! 2 layers for all atoms a better compromise???
#define LAYERS		3	
#define LAYER_LIMIT	0.10	// THEREFORE DISABLED IN CODE!!! but still makes the arrays -> memory!!!

static const int size_nl[LAYERS] = { 100, 200, 400 };

#define SIZE_NT		100
#define SIZE_COI	100
#define SIZE_IPD	50
#define SIZE_IPS	100
#define SIZE_ARC	100

#define INDEX_FLAG 0x8000000				// index of the point
#define ORDER_FLAG 0x4000000				// 0 = starting point, 1 = ending point
#define FLAG_MASK ~(INDEX_FLAG | ORDER_FLAG)

#define BETA 52.0					// beta-angle of the bt4-term...

class eng1_sf;

/*################################################################################################*/

#include "engine.h"
#include "model.h"

#include <vector>
using namespace std;

/*################################################################################################*/

struct eng1_sf_param
{
	f64 vdwrad;
	f64 lenjon;
	f64 wescc;
	f64 wescd;
	f64 dipole1;
	f64 dipole2;
	f64 epsilon1;
	f64 epsilon2;
	f64 epsilon3;
	f64 epsilon4;
	f64 epsilon5;
	f64 epsilon9;
	f64 exp_solv_1n;
	f64 exp_solv_1p;
	f64 exp_solv_2;
	f64 imp_solv_1n;
	f64 imp_solv_1p;
	f64 imp_solv_2;
	f64 solvrad;
	f64 wang;
	f64 wtor1;
	f64 wtor2;
	f64 wrep;
	
	f64 charge_wes[4];
	f64 charge_sasa1[4];
	f64 charge_sasa2[4];
	f64 charge_pKa[9];
	bool charge_acid[9];
	
	f64 rms;	// for random search only... ERROR???
	
	f64 pH;		///< This is the ONLY value that can be modified by user!!!
	
	bool operator<(const eng1_sf_param & p1) const { return (rms < p1.rms); }
};

void eng1_sf_param_SetDefaultValues(eng1_sf_param &);
void eng1_sf_param_MakeCopy(eng1_sf_param &, eng1_sf_param &);

/*################################################################################################*/

class sf_chn		// chain
{
	protected:
	
	vector<sf_res> res_vector;
	
	friend class setup1_sf;
	friend void CopyCRD(model *, engine *, i32u);
	friend void CopyCRD(engine *, model *, i32u);
	friend class eng1_sf;
	
	friend class project;		// FIXME : for ghemical; read access to res_vector???
	friend class ribbon;		// FIXME : for ghemical; read access to res_vector???
	
	public:
	
	sf_chn(void);
	sf_chn(const sf_chn &);
	~sf_chn(void);
};

/*################################################################################################*/

#define SF_STATE_HELIX4		0
#define SF_STATE_STRAND		1
#define SF_STATE_LOOP		2

class sf_res		// residue
{
	protected:
	
	char symbol;
	atom * peptide[5];	// pointers to atoms of the peptide unit.
	
	i32s natm;
	atom * atmr[3];		// pointers to "simplified" atoms.
	i32s loc_varind[3];	// SF-atom indices of "simplified" atoms.
	
	i32u state;
	
	friend class setup1_sf;
	friend void CopyCRD(model *, engine *, i32u);
	friend void CopyCRD(engine *, model *, i32u);
	friend class eng1_sf;
	
	public:
	
	sf_res(void);
	sf_res(char, atom *, atom *, atom *, atom *, atom *, i32s, atom *, atom *, atom *, i32s, i32s, i32s);
	sf_res(const sf_res &);
	~sf_res(void);
	
	i32s GetNumA(void) { return natm; }
	atom * GetRefA(i32u atmi) { return (atmi < 3 ? atmr[atmi] : NULL); }
	
	i32u GetState(void) { return state; }
};

/*################################################################################################*/

class sf_dsb		// disulphide bridge
{
	protected:
	
	i32s chn[2];
	i32s res[2];

	friend class setup1_sf;
	friend class eng1_sf;
	
	friend class project;		// FIXME : for ghemical
	
	public:
	
	sf_dsb(void) { }
	~sf_dsb(void) { }
};

/*################################################################################################*/

class sf_helix4info
{
	protected:
	
	i32u chn;
	i32u res[2];
	
	vector<atom *> ua_donacc;
	vector<atom *> cg_donacc;
	
	friend class setup1_sf;
	friend class project;		// FIXME : for ghemical
	
	public:
	
	sf_helix4info(i32u c, i32u rb, i32u re)
	{
		chn = c;
		res[0] = rb;
		res[1] = re;
	}
	
	~sf_helix4info(void)
	{
	}
	
	i32u GetChn(void) { return chn; }
	i32u GetResBgn(void) { return res[0]; }
	i32u GetResEnd(void) { return res[1]; }
};

class sf_strandinfo
{
	protected:
	
	i32u chn;
	i32u res[2];
	
	public:
	
	sf_strandinfo(i32u c, i32u rb, i32u re)
	{
		chn = c;
		res[0] = rb;
		res[1] = re;
		
		cout << "DEBUG sf_strandinfo ; created record ";
		cout << "c= " << chn << " rb= " << res[0] << " re= " << res[1];
		cout << endl;
	}
	
	~sf_strandinfo(void)
	{
	}
	
	i32u GetChn(void) { return chn; }
	i32u GetResBgn(void) { return res[0]; }
	i32u GetResEnd(void) { return res[1]; }
};

class sf_strandpair
{
	protected:
	
	i32u chn[2];	// the first residues in chain direction...
	i32u res[2];	// the first residues in chain direction...
	
	i32u length;
	bool parallel;
	
	friend class setup1_sf;
	friend class project;		// FIXME : for ghemical
	
	public:
	
	vector<atom *> ua_donacc;
	
	vector<atom *> cg_straight;
	vector<atom *> cg_crossed;
	
	public:
	
	sf_strandpair(i32u c1, i32u r1, i32u c2, i32u r2, i32u l, bool p)
	{
		chn[0] = c1; res[0] = r1;
		chn[1] = c2; res[1] = r2;
		
		length = l;
		parallel = p;
	}
	
	~sf_strandpair(void)
	{
	}
	
	bool ContainsPair(i32u c1, i32u r1, i32u c2, i32u r2)
	{
		for (i32s n1 = 0;n1 < (i32s) length;n1++)
		{
			for (i32s dir = 0;dir < 2;dir++)
			{
				if (c1 != chn[dir] || c2 != chn[!dir]) continue;
				
				i32s tmpr[2]; tmpr[dir] = ((i32s) res[0]) + n1;
				tmpr[!dir] = ((i32s) res[1]) + (parallel ? +n1 : -n1);
				
				if (r1 != (i32u) tmpr[0] || r2 != (i32u) tmpr[1]) continue;
				
				return true;
			}
		}
		
		return false;
	}
	
	bool IsParallel(void) { return parallel; }
	bool IsAntiParallel(void) { return !parallel; }
};

/*################################################################################################*/

/// A setup class for SF submodels.

class setup1_sf : virtual public setup
{
//	protected:
	public:
	
	eng1_sf_param prm;
	
	protected:
	
	bool united_atoms;
	bool exp_solv;
	
	vector<sf_chn> chn_vector;
	vector<sf_dsb> dsb_vector;
	
	vector<sf_helix4info> hi_vector;
	vector<sf_strandinfo> si_vector;
	vector<sf_strandpair> sp_vector;
	
	friend void CopyCRD(model *, engine *, i32u);
	friend void CopyCRD(engine *, model *, i32u);
	friend class eng1_sf;
	
	friend class setup_druid;	// FIXME:  for ghemical		// for BBB
	friend class setup_dialog;	// FIXME:  for ghemical		// for HEAD
	friend class project;		// FIXME : for ghemical; read access to chn_vector???
	friend class ribbon;		// FIXME : for ghemical; read access to chn_vector???
	
	public:
	
	setup1_sf(model *, bool = true);
	~setup1_sf(void);
	
	void UpdateAtomFlags(void);		// virtual
	
	void GetReducedCRD(iter_al *, vector<i32s> &, fGL *, i32u);
	
	void StorePStatesToModel(eng1_sf *);
	
	static i32u static_GetEngineCount(void);
	static i32u static_GetEngineIDNumber(i32u);
	static const char * static_GetEngineName(i32u);
	static const char * static_GetClassName(void);
	
	i32u GetEngineCount(void);		// virtual
	i32u GetEngineIDNumber(i32u);		// virtual
	const char * GetEngineName(i32u);	// virtual
	const char * GetClassName(void);	// virtual
	
	engine * CreateEngineByIndex(i32u);	// virtual
};

/*################################################################################################*/

struct sf_bt1_data		// saved distance results.
{
	f64 data1;			// len????
	f64 data2[2][3];		// dlen????
};

struct sf_bt2_data		// saved angle results.
{
	f64 data1;			// ang????
	f64 data2[3][3];		// dang????
};

struct sf_nonbonded_lookup
{
	char s1; i32u a1;
	char s2; i32u a2;
	
	f64 opte;
};

#define TTYPE_LOOP	0x00
#define TTYPE_HELIX	0x01
#define TTYPE_STRAND	0x02

#define TTYPE_SIDE	0x10
#define TTYPE_BRIDGE	0x11
#define TTYPE_TERM	0x12

struct sf_bt1			// virtual-bond stretching.
{
	i32s atmi[2];
	
	f64 opt; f64 fc;
	
	i32s GetIndex(i32s p1, bool p2)
	{
		return atmi[p2 ? p1 : !p1];
	}
};

struct sf_bt2			// virtual-angle bending.
{
	i32s atmi[3];
	
	i32s index1[2];
	bool dir1[2];
	
	i32s ttype;
	
	f64 opt; f64 fc[2];
};

struct sf_bt3			// main-chain torsion/dipole terms.
{
	i32s atmi[4];
	
	i32s index2[2];
	i32s index1[4];
	bool dir1[4];
	
	i32s tor_ttype;
	
	f64 torc[3];
	f64 tors[3];
	
	// dipole term starts here... dipole term starts here... dipole term starts here...
	// dipole term starts here... dipole term starts here... dipole term starts here...
	// dipole term starts here... dipole term starts here... dipole term starts here...
	
	f64 pbdd;
	
	f64 bv[2][3];
	f64 dbv[2][3][3][3];
	
	f64 dv[3];
	f64 ddv[4][3][3];
	
	i32s dip_ttype;
	
	bool skip;
	
	f64 dipc[3];
	f64 dips[3];
	f64 dipk[2];
};

struct sf_bt4			// 1st side-chain virtual-atom terms.
{
	i32s index1;
	i32s index2;
	
	f64 opt; f64 fc;
	
	f64 fscos[3];
	f64 fssin[3];
};

struct sf_nbt1			// nonbonded terms.
{
	i32s atmi[2];
	
	f64 data[2];
	
	bool operator==(const sf_nbt1 & p1) const
	{
		if ((atmi[0] == p1.atmi[0]) && (atmi[1] == p1.atmi[1])) return true;
		if ((atmi[0] == p1.atmi[1]) && (atmi[1] == p1.atmi[0])) return true;
		
		return false;
	}
};

struct sf_nbt3_nd		///< SASA neighbor data.
{
	i32s index;
	f64 distance;
	
	// these are sorted in reverse order, from large to small...
	// these are sorted in reverse order, from large to small...
	// these are sorted in reverse order, from large to small...
	
	bool operator<(const sf_nbt3_nd & p1) const
	{
		return (distance > p1.distance);
	}
};

struct sf_nbt3_ipd		///< SASA intersection point data.
{
	f64 angle;
	i32u ipdata;
	
	bool operator<(const sf_nbt3_ipd & p1) const
	{
		return (angle < p1.angle);
	}
};

struct sf_nbt3_nl		///< SASA neighbor list.
{
	i32s index_count;
	i32s * index;
};

struct sf_nbt3_coi		///< SASA circle of intersection.
{
	i32s index;
	
	i32s ipd_count;
	sf_nbt3_ipd ipdt[SIZE_IPD];
	
	f64 refv[3];
	
	f64 dist;
	f64 dv[3]; f64 ddv[3][3];
	
	f64 g; f64 dg[3];
	f64 ct; f64 dct[3];
	
	bool flag;
	
	void AddIPD(f64 * p1, i32u p2)
	{
		ipdt[ipd_count].ipdata = p2;
		
		if (!ipd_count)
		{
			f64 t1a[3];
			t1a[0] = dv[0] * p1[0];
			t1a[1] = dv[1] * p1[1];
			t1a[2] = dv[2] * p1[2];
			
			f64 t1b = t1a[0] + t1a[1] + t1a[2];
			
			refv[0] = p1[0] - dv[0] * t1b;
			refv[1] = p1[1] - dv[1] * t1b;
			refv[2] = p1[2] - dv[2] * t1b;
			
			f64 t1c = sqrt(refv[0] * refv[0] + refv[1] * refv[1] + refv[2] * refv[2]);
			refv[0] /= t1c; refv[1] /= t1c; refv[2] /= t1c;
			
			ipdt[ipd_count].angle = 0.0;
		}
		else
		{
			f64 t1a[3];
			t1a[0] = dv[0] * p1[0];
			t1a[1] = dv[1] * p1[1];
			t1a[2] = dv[2] * p1[2];
			
			f64 t1b = t1a[0] + t1a[1] + t1a[2];
			
			f64 t2a[3];
			t2a[0] = p1[0] - dv[0] * t1b;
			t2a[1] = p1[1] - dv[1] * t1b;
			t2a[2] = p1[2] - dv[2] * t1b;
			
			f64 t1c = sqrt(t2a[0] * t2a[0] + t2a[1] * t2a[1] + t2a[2] * t2a[2]);
			t2a[0] /= t1c; t2a[1] /= t1c; t2a[2] /= t1c;
			
			f64 t1d = refv[0] * t2a[0] + refv[1] * t2a[1] + refv[2] * t2a[2];
			if (t1d < -1.0) t1d = -1.0;	// domain check...
			if (t1d > +1.0) t1d = +1.0;	// domain check...
			
			f64 t9a = acos(t1d);
			
			f64 t3a[3];
			t3a[0] = dv[1] * t2a[2] - dv[2] * t2a[1];
			t3a[1] = dv[2] * t2a[0] - dv[0] * t2a[2];
			t3a[2] = dv[0] * t2a[1] - dv[1] * t2a[0];
			
			f64 t9b = refv[0] * t3a[0] + refv[1] * t3a[1] + refv[2] * t3a[2];
			
			if (t9b < 0.0) ipdt[ipd_count].angle = -t9a;
			else ipdt[ipd_count].angle = +t9a;
		}
		
		ipd_count++;
		if (ipd_count >= SIZE_IPD) { cout << "BUG: IPD overflow!!!" << endl; exit(EXIT_FAILURE); }
	}
};

struct sf_nbt3_ips		///< SASA intersection points.
{
	i32s coi[2];
	
	f64 ipv[2][3];
	f64 dipv[2][2][3][3];
};

struct sf_nbt3_arc		///< SASA positively oriented arc.
{
	i32s coi;
	i32s index[2][2];
	
	i32u ipdata[2];
	
	f64 tv[2][3];
	f64 dtv[2][2][3][3];
	
	bool flag;
};

/*################################################################################################*/

class eng1_sf : virtual public engine_mbp
{
//	protected:
	public:		// FIX_ME
	
	i32u * l2g_sf;		///< The local-to-global lookup table.
	i32s * index_chn;	///< This maps the (local) atoms to setup1_sf chains/residues.
	i32s * index_res;	///< This maps the (local) atoms to setup1_sf chains/residues.
	i32s num_solvent;
	
	eng1_sf_param * myprm;
	bool use_explicit_solvent;
	bool use_implicit_solvent;
	
	vector<sf_bt1> bt1_vector;
	vector<sf_bt2> bt2_vector;
	vector<sf_bt3> bt3_vector;
	vector<sf_bt4> bt4_vector;
	
	sf_bt1_data * bt1data;
	sf_bt2_data * bt2data;
	
	vector<sf_nbt1> nbt1_vector;
	
	f64 * mass;		// MD???
	f64 * vdwr;
	f64 * charge1;		// actual charges.
	f64 * charge2;		// neutralized charges.
	
	i32s * dist1; f64 * dist2;
	sf_nbt3_nl * nbt3_nl[LAYERS];
	
	f64 * vdwr1[LAYERS];
	f64 * vdwr2[LAYERS];
	f64 * sasaE[LAYERS];
	
	fGL * solv_exp[LAYERS];
	
	public:
	
	f64 energy_bt1;
	f64 energy_bt2;
	f64 energy_bt3a;
	f64 energy_bt3b;
	f64 energy_bt4a;
	f64 energy_bt4b;
	f64 energy_nbt1a;
	f64 energy_nbt1b;
	f64 energy_nbt1c;
	f64 energy_nbt2a;
	f64 energy_nbt2b;
	f64 energy_nbt2c;
	f64 energy_nbt3a;
	f64 energy_nbt3b;
	f64 energy_pnlt;
	
	i32s * tmp_vartab;
	f64 * tmp_parames;
	f64 * tmp_paramsa1;
	f64 * tmp_paramsa2;
	f64 * tmp_newpKa;
	
	f64 constraints;
	
	public:
	
	eng1_sf(setup *, i32u, bool, bool);
	virtual ~eng1_sf(void);
	
	void SetupCharges(void);
	void GetChgGrpVar(i32s, i32s &, i32s &);
	
	virtual void Compute(i32u);	// virtual
	
	i32s FindTorsion(atom *, atom *, atom *, atom *) { return NOT_DEFINED; }	// virtual
	bool SetTorsionConstraint(i32s, f64, f64, bool) { return false; }		// virtual
	
	i32s GetOrbitalCount(void) { return 0; }	// virtual
	f64 GetOrbitalEnergy(i32s) { return 0.0; }	// virtual
	
	i32s GetElectronCount(void) { return 0; }	// virtual
	
	void SetupPlotting(void) { }		// virtual
	
	fGL GetVDWSurf(fGL *, fGL *);		// virtual
	
	fGL GetESP(fGL *, fGL *);		// virtual
	
	fGL GetElDens(fGL *, fGL *) { return 0.0; }	// virtual
	
	fGL GetOrbital(fGL *, fGL *) { return 0.0; }	// virtual
	fGL GetOrbDens(fGL *, fGL *) { return 0.0; }	// virtual
	
	protected:
	
	bool InitNBT1(sf_nbt1 *, vector<sf_nonbonded_lookup> &);
	void InitLenJon(sf_nbt1 *, f64, f64);
	
	void ComputeBT1(i32u);
	void ComputeBT2(i32u);
	void ComputeBT3(i32u);
	void ComputeBT4(i32u);
	
	void ComputeNBT1(i32u);
	void ComputeNBT2(i32u);
	void ComputeNBT3(i32u);
};

/*################################################################################################*/

#endif	// ENG1_SF_H

// eof
