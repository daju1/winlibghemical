// look_md_results.cpp : Defines the entry point for the console application.
//
#include "stdafx.h"
#include "..\..\long_stream.h"

#include "../example2.h"

#include "..\..\notice.h"

#include "..\..\eng1_mm_tripos52.h"
#include "..\..\eng1_mm_default.h"

#include "..\..\geomopt.h"
#include "..\..\moldyn.h"

#include <windows.h>

#include "../../../../consol/filedlg.h"
#include "readfile.h"

#define _USE_WINGHEMICAL_ 0
#if _USE_WINGHEMICAL_
#include "../../../../src/win32_app.h"
#include "../../src/utility.h"
#include "../../../consol/winghemical.h"
#endif

HINSTANCE hInst;


double calcMathOzh(const vector<double> & dF, const int& j, const int& wind, int& number)
{
	double m = 0.0;
	int j_wind = j*wind;
	for (int i = (j-1)*wind; i < j_wind; i++)
	{
		m += dF[i];
		++number;
	}
	m /= wind;

	return m;
}

double calcLinearTrendParameterA(const vector<double> & dF, const int& j, const int& wind, int& number)
{
	double sumY = 0.0;
	double sumx = 0.0;
	double sumYx = 0.0;
	double sumxx = 0.0;

	int j_wind = j*wind;
	for (int i = (j-1)*wind; i < j_wind; i++)
	{
		double x = i;
		double Y = dF[i];

		sumY += Y;
		sumx += x;

		sumxx += x*x;
		sumYx += Y*x;

		++number;
	}

	double n = wind;


	double a = (n*sumYx - sumx*sumY) / (n*sumxx - sumx*sumx);

	return a;
}

typedef double (*calcfun)(const vector<double> & dF, const int& j, const int& wind, int& number);



void Statistika (const vector<double> & dF, const char * name, int N, calcfun fun)
{
	int rows = dF.size();
	vector<double> dF_means;	
	int wind = int(rows / N);

	double mat_ozh = 0.0;

	int number = 0;
	
	for (int j = 1; j <= N; j++)
	{
#if 1
		double m = (*fun)(dF, j, wind, number);
#else
		double m = 0.0;
		int j_wind = j*wind;
		for (int i = (j-1)*wind; i < j_wind; i++)
		{
			m += dF[i];
			++number;
		}
		m /= wind;
#endif
		//
		dF_means.push_back(m);
		mat_ozh += m;
	}
	mat_ozh /= N;
	double D = 0.0;

	for (int j = 0; j < N; j++)
	{
		double d = dF_means[j] - mat_ozh;
		D += d*d;
	}
	D /= N - 1;
	
	double sigma = sqrt(D / N);

	printf("\nStatistika %s N = %d ", name, N);
	printf("mat_ozh = %f ", mat_ozh);
	printf("sigma = %f wind=%d calculed=%d from all=%d ", sigma, wind, number, rows);
}

#define WRITE_LOCKED_FORCES 0
#define WRITE_WORKED_FORCES 0

int Look_txt()
{
	TCHAR filter[] =     TEXT("Ghemical MD results File (*.txt)\0*.txt\0")
						 TEXT("All Files (*.*)\0*.*\0");
	TCHAR fpath[1024];
	TCHAR filename[1024];

	sprintf(filename, "\0");
	{
		DWORD nFilterIndex;

		vector<string> names;
		vector<string> *pnames = &names;
		vector<vector<double> > vectors;
		vectors.reserve(2000000);

		while (OpenFileDlg(0, filter, fpath, nFilterIndex) == S_OK)
		{		
			ReadDatFile(NULL, fpath, filename, &vectors, pnames);
			pnames = NULL;

			printf("\nfilename %s\n\n", filename);

			int cols = names.size();
			int rows = vectors.size();
			
#if WRITE_LOCKED_FORCES
			int cMom = 4 - 1;
			int cVx = 5 - 1;
			int cFxup = 14 - 1;
			int cFxdw = 17 - 1;

			int cVxup = 8 - 1;
			int cVxdw = 11 - 1;
#endif
#if WRITE_WORKED_FORCES
			int cMom = 4 - 1;

			int cVx = 5 - 1;
			int cVxup = 14 - 1;
			int cVxdw = 17 - 1;

			int cVx_wk_up = 8 - 1;
			int cVx_wk_dw = 11 - 1;

			int cFx_wk_up = 20 - 1;
			int cFx_wk_dw = 23 - 1;

#endif

			vector<double> means(cols, 0.0);


			printf("vectors.size() = %d\n",rows);
			printf("names.size() = %d\n", cols);

			for (vector<vector<double> >::iterator it = vectors.begin();
			it != vectors.end(); it++)
			{
				for (int c = 0; c < cols; c++)
				{
					means[c] += (*it).operator [](c);
				}
			}

			for (int c = 0; c < cols; c++)
			{
				means[c] /= rows;
				printf("mean(%s) = %f\n", names[c].c_str(), means[c]);
			}

#if WRITE_LOCKED_FORCES || WRITE_WORKED_FORCES
			int r0 = 0;

			cout << "enter r0\n";
			cin >> r0;
#endif

#if WRITE_LOCKED_FORCES
			vector<double> dF(rows-r0);
			for (int r = r0; r < rows; r++)
			{
				dF[r-r0] = vectors[r][cFxup] - vectors[r][cFxdw];
			}

			Statistika (dF, "dF");

			vector<double> Mom(rows-r0);
			for (r = r0; r < rows; r++)
			{
				Mom[r-r0] = vectors[r][cMom];
			}

			Statistika (Mom, "Mom");

			vector<double> dV(rows-r0);
			for (r = r0; r < rows; r++)
			{
				dV[r-r0] = vectors[r][cVxup] - vectors[r][cVxdw];
			}

			Statistika (dV, "dV");

			vector<double> Vx(rows-r0);
			for (r = r0; r < rows; r++)
			{
				Vx[r-r0] = vectors[r][cVx];
			}

			Statistika (Vx, "Vx");
#endif
#if WRITE_WORKED_FORCES
			vector<double> dF_wk(rows-r0);
			for (int r = r0; r < rows; r++)
			{
				dF_wk[r-r0] = vectors[r][cFx_wk_up] - vectors[r][cFx_wk_dw];
			}

			Statistika (dF_wk, "dF_wk");


			vector<double> dV_wk(rows-r0);
			for (r = r0; r < rows; r++)
			{
				dV_wk[r-r0] = vectors[r][cVx_wk_up] - vectors[r][cVx_wk_dw];
			}

			Statistika (dV_wk, "dV_wk");

			//if (!worked[n1])
			vector<double> Mom(rows-r0);
			for (r = r0; r < rows; r++)
			{
				Mom[r-r0] = vectors[r][cMom];
			}

			Statistika (Mom, "Mom");

			vector<double> dV(rows-r0);
			for (r = r0; r < rows; r++)
			{
				dV[r-r0] = vectors[r][cVxup] - vectors[r][cVxdw];
			}

			Statistika (dV, "dV");

			vector<double> Vx(rows-r0);
			for (r = r0; r < rows; r++)
			{
				Vx[r-r0] = vectors[r][cVx];
			}

			Statistika (Vx, "Vx");
#endif
		}
	}
	/*else
	{
		DWORD nFilterIndex;
		if (SaveFileDlg(0, filename, filter, nFilterIndex) == S_OK)
		{
			SetDlgItemText(ref->hDlg,IDC_EDIT_TRAJFILE2, filename);
		}	
	}*/
	
	printf("Hello World!\n");
	return 0;

}


size_t GetTrajectoryHeaderSize(bool extended_trajectory)
{
	if (extended_trajectory)
		return (8 + 2 * sizeof(int) + sizeof(f64));

	return (8 + 2 * sizeof(int));
}

size_t GetTrajectoryEnergySize()
{
	return (2 * sizeof(float));
}

size_t GetTrajectoryFrameSize(bool extended_trajectory, int natoms)
{
	if (extended_trajectory)
		return (GetTrajectoryEnergySize() + 12 * natoms * sizeof(float));

	return (GetTrajectoryEnergySize() + 3 * natoms * sizeof(float));
}

void Load_traj(int ind1, int ind2, int dim, vector<double> & X)
{
	//print_lib_intro_notice(cout);
	//print_copyright_notice(cout);
	cout << endl;

	// example #2: open a file, run a geometry optimization, and save a file.
	// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
	char infile_name[1048];
	DWORD nFilterIndex;
	if (OpenFileDlg(0, "Ghemical Project File (*.gpr)\0*.gpr\0All files \0*.*\0", 
		infile_name, nFilterIndex) 
		!= S_OK)
	{
		return;
	}


	cout << "trying to open a file " << infile_name << " ; ";

	ifstream ifile;
	ifile.open(infile_name, ios::in);

	if (ifile.fail())
	{
		cout << "failed!!!" << endl;

		ifile.close();
		return;
	}

	cout << "ok!!!" << endl;
#if _USE_WINGHEMICAL_

	//готовим классы для 3D визуализации
	hInst = GetModuleHandle(NULL);

	singleton_cleaner<win_app> app_cleaner;
	win_app * app = win_app::GetInstance();  
	// new win_app() here 
	// -> 	SetNewProject(); 
	// -> new win_project(* win_class_factory::GetInstance());
	// -> new win_class_factory() : graphics_class_factory()
	// 

	// win_project()
	// -> CreateProjectView() -> new win_project_view(prj2) -> LightAdded(); ObjectAdded();

	// win_project()
	// -> AddGraphicsView(NULL, false);

	// project::AddGraphicsView()
	// -> win_class_factory::ProduceGraphicsView() 
	// -> new win_graphics_view(prj2, cam);
	// -> BeginWinGhemical(); view_widget = CreateWinGhemical(this); SendMessage(view_widget, WM_COMMAND, ID_FILE_INITOG2, 0); ConfigureHandler(view_widget);
	app_cleaner.SetInstance(app);

	app->GetWinProject()->SetApp(app);
	//app->work(argc, argv); //
	ReadGPR(* app->GetWinProject(), ifile, false, false);
	ifile.close();

	{
		HANDLE hThread = NULL;
		{
			setup * su = app->GetWinProject()->GetCurrentSetup();
			static jobinfo_MolDyn_tst ji;

			ji.prj = app->GetWinProject();
			ji.md = moldyn_tst_param(su);

			app->GetWinProject()->start_job_MolDyn_tst(& ji);
		}
	}

	StartWinGhemical(NULL);

#else
	model * mdl = new model();
	ReadGPR(* mdl, ifile, false, false);
	ifile.close();
#endif
	char fn[512];
	if (OpenFileDlg(NULL, "Ghemical Trajectory File (*.traj)\0*.traj\0All files \0*.*\0", fn, nFilterIndex) == S_OK)
	{
		cout << "trying to open \"" << fn << "\"." << endl;

		ifstream * trajfile;

		//mdl->OpenTrajectory(filename);
		//{
			i32s traj_num_atoms = mdl->GetAtomCount();
			/*	for (iter_al it1 = GetAtomsBegin();it1 != GetAtomsEnd();it1++)
			{
				if (!((* it1).flags & ATOMFLAG_IS_HIDDEN)) traj_num_atoms++;
			}	
			*/
			
			trajfile = new ifstream(fn, ios::in | ios::binary);
			//trajfile->seekg(8, ios::beg);	// skip the file id...
			char file_id[10];
			trajfile->read((char *) file_id, 8);
			bool extended_trajectory = false;
			if (0 == ::strncmp(file_id, "traj_v11", 8))
				extended_trajectory = true;

			int natoms;
			trajfile->read((char *) & natoms, sizeof(natoms));
			
			if (natoms != traj_num_atoms)
			{
				printf("natoms %d != traj_num_atoms %d\n", natoms, traj_num_atoms);
//				ErrorMessage("The trajectory is incompatible with the current structure/setup!!!");
//				PrintToLog("incompatible file : different number of atoms!\n");
				goto e_close_traj;
			}

			i32s total_traj_frames;			
			trajfile->read((char *) & total_traj_frames, sizeof(total_traj_frames));

			float tstep = 0.0;
			if (extended_trajectory)
				trajfile->read((char *) & tstep, sizeof(tstep));


			
			//char mbuff1[256]; strstream str1(mbuff1, sizeof(mbuff1));
			//str1 << "the trajectory file contains " << total_traj_frames << " frames." << endl << ends;
			//PrintToLog(mbuff1);

			//current_traj_frame = 0;
		//}

		atom * mt_a1;	// measure_tool
		atom * mt_a2;	// measure_tool
		atom * mt_a3;	// measure_tool			

		float ekin;
		float epot;

		X.resize(total_traj_frames);

		//i32s max_frames = app->GetWinProject()->GetTotalFrames();
		for (i32s loop = 0;loop < total_traj_frames; loop++)
		{
			//app->GetWinProject()->SetCurrentFrame(loop);
			//this->ReadTrajectoryFrame();
			//void project::ReadTrajectoryFrame(void)
			//{
				i32s place = GetTrajectoryHeaderSize(extended_trajectory);						// skip the header...
				place += GetTrajectoryFrameSize(extended_trajectory, natoms) * loop;		// get the correct frame...
				//place += GetTrajectoryEnergySize();							// skip epot and ekin...
				
				trajfile->seekg(place, ios::beg);

				trajfile->read((char *) & ekin, sizeof(ekin));
				trajfile->read((char *) & epot, sizeof(epot));

				i32s ind = 0;
				mt_a1 = mt_a2 = mt_a3 = NULL;		



				for (iter_al it1 = mdl->GetAtomsBegin(); it1 != mdl->GetAtomsEnd(); it1++)
				{
				//	if ((* it1).flags & ATOMFLAG_IS_HIDDEN) continue;	// currently all coordinates are written...
					float t1a;
					fGL cdata[3];
					for (i32s t4 = 0;t4 < 3;t4++)
					{
						trajfile->read((char *) & t1a, sizeof(t1a));
						cdata[t4] = t1a;
					}
					if (extended_trajectory)
					{
						for (i32s t4 = 0;t4 < 3;t4++){
							trajfile->read((char *) & t1a, sizeof(t1a));
							//vdata[t4] = t1a;
						}
						for (i32s t4 = 0;t4 < 3;t4++){
							trajfile->read((char *) & t1a, sizeof(t1a));
							//adata[t4] = t1a;
						}
						for (i32s t4 = 0;t4 < 3;t4++){
							trajfile->read((char *) & t1a, sizeof(t1a));
							//fdata[t4] = t1a;
						}
					}
					
					(* it1).SetCRD(0, cdata[0], cdata[1], cdata[2]);

					if (ind == ind1)
					{
						mt_a1 = &(* it1);
					}

					if (ind == ind2)
					{
						mt_a2 = &(* it1);
					}

					/*if (ind == indc)
					{
						mt_a3 = &(* it1);
					}*/


					ind++;
				}
				//this->UpdateAllGraphicsViews();
			//}

				fGL coordinate;

				if (mt_a1 && mt_a2 && dim >=0 && dim < 3)
				{
					const fGL * p1 = mt_a1->GetCRD(0);
					const fGL * p2 = mt_a2->GetCRD(0);
// my measure
//cout  << "el = " << mt_a1->el.GetSymbol() << " " << mt_a1->el.GetAtomicNumber() << " x = " << p1[0] << " y = " << p1[1] << " z = " << p1[2] << endl;
					coordinate = p1[dim]-p2[dim];
				}
				else
					coordinate = 0;

				X[loop] = coordinate;

			//ref->this->UpdateAllGraphicsViews(true);
			//::Sleep(100);
			//void * udata = convert_cset_to_plotting_udata(this, 0);
			//f64 value = coordinate;
			//plot->AddData(loop, value, udata);

			//mt_a1 = mt_a2 = mt_a3 = NULL;
		}
e_close_traj:
		//app->GetWinProject()->CloseTrajectory();
		if (trajfile != NULL)
		{
			trajfile->close();
			delete trajfile;

			trajfile = NULL;
		}
	}
}

void Look_traj()
{
	//make_plot_crd_diff 110 1605 0
	vector<double> X;
	Load_traj(110, 1605, 0, X);

	int szX = X.size();

	if (szX == 0)
		return;

	vector<double> diffX(szX-1);
	for (int i = 0; i < szX-1; ++i)
	{
		diffX[i] = X[i+1] - X[i];
	}

	{
		//------------------------------------------------------------------
		char filter[] =     TEXT("Plot Text File (*.txt)\0*.txt\0")
			TEXT("All Files (*.*)\0*.*\0");

		char outfile_name[1048];
		sprintf(outfile_name, "\0");
		DWORD nFilterIndex;
		if (SaveFileDlg(0, outfile_name, filter, nFilterIndex) == S_OK)
		{
			cout << "now saving file " << outfile_name << endl;
			ofstream ofile;
			ofile.open(outfile_name, ios::out);

			for (int i = 0; i < szX-1; ++i)
			{
				ofile << X[i] << "," << diffX[i] << endl;
			}

			ofile.close();
		}
	}


	for (int N = 10; N < 100; N += 5)
		Statistika (diffX, "diffX", N, calcMathOzh);

	for (int N = 10; N < 100; N += 5)
		Statistika (X, "linearTrendParameterA", N, calcLinearTrendParameterA);

}

int main(int argc, char* argv[])
{
	Look_traj();
}

