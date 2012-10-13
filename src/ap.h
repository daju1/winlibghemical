#ifndef AP_H
#define AP_H

#include <stdlib.h>
#include <math.h>
#include <vector>
using namespace std;

/********************************************************************
Œœ»—¿Õ»≈  À¿——Œ¬-Ã¿——»¬Œ¬

 À¿—— ŒƒÕŒÃ≈–ÕŒ√Œ Ã¿——»¬¿:

template<class T>
class template_1d_array
{
public:
    template_1d_array();
    ~template_1d_array();
    template_1d_array(const template_1d_array &rhs);
    const template_1d_array& operator=(const template_1d_array &rhs);

    const T& operator()(int i) const;
    T& operator()(int i);

    void setbounds(int iLow, int iHigh);
    void setcontent( int iLow, int iHigh, const T *pContent );
    T* getcontent();

    int getlowbound(int iBoundNum) const;
    int gethighbound(int iBoundNum) const;
};

template<class T>
class template_2d_array
{
public:
    template_2d_array();
    ~template_2d_array();
    template_2d_array(const template_2d_array &rhs);
    const template_2d_array& operator=(const template_2d_array &rhs);

    const T& operator()(int i1, int i2) const;
    T& operator()(int i1, int i2);

    void setbounds( int iLow1, int iHigh1, int iLow2, int iHigh2 );
    void setcontent( int iLow1, int iHigh1, int iLow2, int iHigh2, const T *pContent );
    T* getcontent();

    int getlowbound(int iBoundNum) const;
    int gethighbound(int iBoundNum) const;
};


********************************************************************/

#define NO_AP_ASSERT
//#define UNSAFE_MEM_COPY

namespace ap
{

class ap_error
{
public:
    static void make_assertion(bool bClause)
        { if(!bClause) throw ap_error(); };
private:
};                      

//*****************************************************************//
//XOR                                                              //
//*****************************************************************//
static inline bool XOR(bool lhs, bool rhs)
{
    return (lhs && !rhs) || ( !lhs && rhs);
}

//*****************************************************************//
//Arrays                                                           //
//*****************************************************************//
template<class T>
class template_1d_array
{
public:
    template_1d_array()
    {
        m_Vec=0;
        m_iVecSize = 0;
    };

    ~template_1d_array()
    {
        if(m_Vec)
            delete[] m_Vec;
    };

    template_1d_array(const template_1d_array &rhs)
    {
        m_iVecSize = rhs.m_iVecSize;
        m_iLow = rhs.m_iLow;
        m_iHigh = rhs.m_iHigh;
        if(rhs.m_Vec)
        {
            m_Vec = new T[m_iVecSize];
            #ifndef UNSAFE_MEM_COPY
            for(int i=0; i<m_iVecSize; i++)
                m_Vec[i] = rhs.m_Vec[i];
            #else
            memcpy(m_Vec, rhs.m_Vec, m_iVecSize*sizeof(T));
            #endif
        }
        else
            m_Vec=0;
    };

    const template_1d_array& operator=(const template_1d_array &rhs)
    {
        m_iLow = rhs.m_iLow;
        m_iHigh = rhs.m_iHigh;
        m_iVecSize = rhs.m_iVecSize;
        if(m_Vec)
            delete[] m_Vec;
        if(rhs.m_Vec)
        {
            m_Vec = new T[m_iVecSize];
            #ifndef UNSAFE_MEM_COPY
            for(int i=0; i<m_iVecSize; i++)
                m_Vec[i] = rhs.m_Vec[i];
            #else
            memcpy(m_Vec, rhs.m_Vec, m_iVecSize*sizeof(T));
            #endif
        }
        else
            m_Vec=0;
        return *this;
    };

    const T& operator()(int i) const
    {
        #ifndef NO_AP_ASSERT
        ap_error::make_assertion(i>=m_iLow && i<=m_iHigh);
        #endif
        return m_Vec[ i-m_iLow ];
    };

    T& operator()(int i)
    {
        #ifndef NO_AP_ASSERT
        ap_error::make_assertion(i>=m_iLow && i<=m_iHigh);
        #endif
        return m_Vec[ i-m_iLow ];
    };

    void setbounds( int iLow, int iHigh )
    {
        if(m_Vec)
            delete[] m_Vec;
        m_iLow = iLow;
        m_iHigh = iHigh;
        m_iVecSize = iHigh-iLow+1;
        m_Vec = new T[m_iVecSize];
    };

    void setcontent( int iLow, int iHigh, const T *pContent )
    {
        setbounds(iLow, iHigh);
        for(int i=iLow; i<=iHigh; i++)
            (*this)(i) = pContent[i-iLow];
    };

    T* getcontent()
    {
        return m_Vec;
    };

    int getlowbound(int iBoundNum) const
    {
        return m_iLow;
    };

    int gethighbound(int iBoundNum) const
    {
        return m_iHigh;
    };
private:
    T         *m_Vec;
    long      m_iVecSize;
    long      m_iLow, m_iHigh;
};

template<class T>
class template_2d_array
{
public:
    template_2d_array()
    {
        m_Vec=0;
        m_iVecSize=0;
    };

    ~template_2d_array()
    {
        if(m_Vec)
            delete[] m_Vec;
    };

    template_2d_array(const template_2d_array &rhs)
    {
        m_iVecSize = rhs.m_iVecSize;
        m_iLow1 = rhs.m_iLow1;
        m_iLow2 = rhs.m_iLow2;
        m_iHigh1 = rhs.m_iHigh1;
        m_iHigh2 = rhs.m_iHigh2;
        m_iConstOffset = rhs.m_iConstOffset;
        m_iLinearMember = rhs.m_iLinearMember;
        if(rhs.m_Vec)
        {
            m_Vec = new T[m_iVecSize];
            #ifndef UNSAFE_MEM_COPY
            for(int i=0; i<m_iVecSize; i++)
                m_Vec[i] = rhs.m_Vec[i];
            #else
            memcpy(m_Vec, rhs.m_Vec, m_iVecSize*sizeof(T));
            #endif
        }
        else
            m_Vec=0;
    };
    const template_2d_array& operator=(const template_2d_array &rhs)
    {
        m_iLow1 = rhs.m_iLow1;
        m_iLow2 = rhs.m_iLow2;
        m_iHigh1 = rhs.m_iHigh1;
        m_iHigh2 = rhs.m_iHigh2;
        m_iConstOffset = rhs.m_iConstOffset;
        m_iLinearMember = rhs.m_iLinearMember;
        m_iVecSize = rhs.m_iVecSize;
        if(m_Vec)
            delete[] m_Vec;
        if(rhs.m_Vec)
        {
            m_Vec = new T[m_iVecSize];
            #ifndef UNSAFE_MEM_COPY
            for(int i=0; i<m_iVecSize; i++)
                m_Vec[i] = rhs.m_Vec[i];
            #else
            memcpy(m_Vec, rhs.m_Vec, m_iVecSize*sizeof(T));
            #endif
        }
        else
            m_Vec=0;
        return *this;
    };

    const T& operator()(int i1, int i2) const
    {
        #ifndef NO_AP_ASSERT
        ap_error::make_assertion(i1>=m_iLow1 && i1<=m_iHigh1);
        ap_error::make_assertion(i2>=m_iLow2 && i2<=m_iHigh2);
        #endif
        //return m_Vec[ (i1-m_iLow1) + (i2-m_iLow2)*(m_iHigh1-m_iLow1+1)];
        //return m_Vec[ m_iConstOffset + i1 +i2*m_iLinearMember];
        //return m_Vec[ (i2-m_iLow2) + (i1-m_iLow1)*(m_iHigh2-m_iLow2+1)];
        return m_Vec[ m_iConstOffset + i2 +i1*m_iLinearMember];
    };

    T& operator()(int i1, int i2)
    {
        #ifndef NO_AP_ASSERT
        ap_error::make_assertion(i1>=m_iLow1 && i1<=m_iHigh1);
        ap_error::make_assertion(i2>=m_iLow2 && i2<=m_iHigh2);
        #endif
        //return m_Vec[ (i1-m_iLow1) + (i2-m_iLow2)*(m_iHigh1-m_iLow1+1)];
        //return m_Vec[ m_iConstOffset + i1 +i2*m_iLinearMember];
        //return m_Vec[ (i2-m_iLow2) + (i1-m_iLow1)*(m_iHigh2-m_iLow2+1)];
        return m_Vec[ m_iConstOffset + i2 +i1*m_iLinearMember];
    };

    void setbounds( int iLow1, int iHigh1, int iLow2, int iHigh2 )
    {
        if(m_Vec)
            delete[] m_Vec;
        m_iVecSize = (iHigh1-iLow1+1)*(iHigh2-iLow2+1);
        m_Vec = new T[m_iVecSize];
        m_iLow1  = iLow1;
        m_iHigh1 = iHigh1;
        m_iLow2  = iLow2;
        m_iHigh2 = iHigh2;
        //m_iConstOffset = -m_iLow1-m_iLow2*(m_iHigh1-m_iLow1+1);
        m_iConstOffset = -m_iLow2-m_iLow1*(m_iHigh2-m_iLow2+1);
        //m_iLinearMember = (m_iHigh1-m_iLow1+1);
        m_iLinearMember = (m_iHigh2-m_iLow2+1);
    };

    void setcontent( int iLow1, int iHigh1, int iLow2, int iHigh2, const T *pContent )
    {
        setbounds(iLow1, iHigh1, iLow2, iHigh2);
        for(int i=0; i<m_iVecSize; i++)
            m_Vec[i]=pContent[i];
    };

    T* getcontent()
    {
        return m_Vec;
    };

    int getlowbound(int iBoundNum) const
    {
        return iBoundNum==1 ? m_iLow1 : m_iLow2;
    };

    int gethighbound(int iBoundNum) const
    {
        return iBoundNum==1 ? m_iHigh1 : m_iHigh2;
    };
private:
    T           *m_Vec;
    long        m_iVecSize;
    long        m_iLow1, m_iLow2, m_iHigh1, m_iHigh2;
    long        m_iConstOffset, m_iLinearMember;
};


typedef template_1d_array<int>    integer_1d_array;
typedef template_1d_array<double> real_1d_array;
typedef template_1d_array<int>   boolean_1d_array;
typedef template_2d_array<int>    integer_2d_array;
typedef template_2d_array<double> real_2d_array;
typedef template_2d_array<int>   boolean_2d_array;


//*****************************************************************//
//Routines and constants                                             //
//*****************************************************************//
static double machineepsilon = 5E-16;
static double maxrealnumber = 1E300;
static double minrealnumber = 1E-300;

static double sign(double x)
{
    if( x>0 ) return  1.0;
    if( x<0 ) return -1.0;
    return 0;
}

static double randomreal()
{
    int i = rand();
    while(i==RAND_MAX)
        i =rand();
    return double(i)/double(RAND_MAX);
}

static int randominteger(int max)
{  return rand()%max; }

static double round(double x)
{ return floor(x+0.5); }

static double trunc(double x)
{ return x>0 ? floor(x) : ceil(x); }

static double pi()
{ return M_PI; }

static double sqr(double x)
{ return x*x; }

};//namespace ap


#endif
