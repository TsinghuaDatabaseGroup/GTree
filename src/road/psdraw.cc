/* ----------------------------------------------------------------------------
    Author: Ken C. K. Lee
    Email:  cklee@cse.psu.edu
    Web:    http://www.cse.psu.edu/~cklee
    Date:   Nov, 2007
---------------------------------------------------------------------------- */

#include "psdraw.h"
#include <iostream>
#include <math.h>

PSDraw::PSDraw(const char* a_filename,
               float a_cwminx, float a_cwminy, float a_cwmaxx, float a_cwmaxy,
               int a_minx, int a_miny, int a_maxx, int a_maxy):
m_cwminx(a_cwminx),
m_cwmaxx(a_cwmaxx),
m_cwminy(a_cwminy),
m_cwmaxy(a_cwmaxy),
m_minx(a_minx),
m_miny(a_miny),
m_maxx(a_maxx),
m_maxy(a_maxy),
m_xscale((a_maxx - a_minx) / (a_cwmaxx - a_cwminx)),
m_yscale((a_maxy - a_miny) / (a_cwmaxy - a_cwminy))
{
    m_f.open(a_filename, fstream::out);
    m_f << "%!PS-Adobe-3.0 EPSF-3.0" << endl;
	m_f << "%%Creator: PSDraw" << endl;
	m_f << "%%Title: PSDraw" << endl;
	m_f << "%%CreationDate: TODAY" << endl;
	m_f << "%%DocumentData: Clean7Bit" << endl;
	m_f << "%%Origin: 0 0" << endl;
	m_f << "%%BoundingBox: ";
    m_f << m_minx << " " << m_miny << " " << m_maxx << " " << m_maxy << endl;
	m_f << "%%LanguageLevel: 2" << endl;
	m_f << "%%Pages: 1" << endl;
	m_f << "%%Page: 1 1" << endl;
    m_f << "%%Orientation: Portrait" << endl;
    m_f << endl;
    m_f << "%%BeginDefaults" << endl;
    m_f << "%%PageBoundingBox: 0 0 600 600" << endl;
    m_f << "%%ViewingOrientation: 1 0 0 1" << endl;
    m_f << "%%EndDefaults" << endl;

}

PSDraw::~PSDraw()
{
	m_f << "%%EOF" << endl;
    m_f << "%%EndDocument" << endl;
    m_f << "%%Trailer" << endl;
}

void PSDraw::point(float a_x, float a_y,
                   float a_lineGrayScale)
{
    int x = (int)ceil((a_x - m_cwminx) * m_xscale + m_minx);
    int y = (int)ceil((a_y - m_cwminy) * m_yscale + m_miny);

    //m_f << "%% draw point at " << a_x << " " << a_y << endl;
	m_f << "newpath" << endl;
	m_f << x << " " << y << " moveto" << endl;
	m_f << x << " " << y << " 1 0 360 arc" << endl;
	m_f << "closepath" << endl;
	m_f << "gsave" << endl;
	m_f << a_lineGrayScale << " setgray" << endl;
	//m_f << "0 0 0 setrgbcolor" << endl;
	m_f << "fill" << endl;
    m_f << "%%" << endl;
}

void PSDraw::line(float a_xstart, float a_ystart,
                  float a_xend, float a_yend,
                  float a_lineGrayScale, int width)
{
    int xstart = (int)ceil((a_xstart - m_cwminx) * m_xscale + m_minx);
    int ystart = (int)ceil((a_ystart - m_cwminy) * m_yscale + m_miny);
    int xend   = (int)ceil((a_xend - m_cwminx) * m_xscale + m_minx);
    int yend   = (int)ceil((a_yend - m_cwminy) * m_yscale + m_miny);

    int xdiff = xend - xstart;
    int ydiff = yend - ystart;
    xdiff = xdiff > 0 ? xdiff : -xdiff;
    ydiff = ydiff > 0 ? ydiff : -ydiff;

    if (xdiff <= 0 && ydiff <= 0)
    {
        //point(a_xstart,ystart,a_lineGrayScale);
        return;
    }
    

    //m_f << "%% draw line (";
    //m_f << xstart << "," << ystart << "),(";
    //m_f << xend << "," << yend << ")" << endl;
	m_f << "newpath" << endl;
	m_f << xstart << " " << ystart << " moveto" << endl;
	m_f << xend << " " << yend << " lineto" << endl;

    m_f << width << " setlinewidth" << endl;
	m_f << a_lineGrayScale << " setgray" << endl;
	m_f << "stroke" << endl;
    m_f << "%%" << endl;
}

void PSDraw::box(float a_xmin, float a_ymin,
                 float a_xmax, float a_ymax,
                 float a_lineGrayScale)
{
	int xmin = (int)ceil((a_xmin - m_cwminx) * m_xscale + m_minx);
	int ymin = (int)ceil((a_ymin - m_cwminy) * m_yscale + m_miny);
	int xmax = (int)ceil((a_xmax - m_cwminx) * m_xscale + m_minx);
	int ymax = (int)ceil((a_ymax - m_cwminy) * m_yscale + m_miny);

    //m_f << "%% draw box covering (";
    //m_f << a_xmin << "," << a_ymin << "),(";
    //m_f << a_xmax << "," << a_ymax << ")" << endl;
	m_f << "newpath" << endl;
	m_f << xmin << " " << ymin << " moveto" << endl;
	m_f << xmin << " " << ymax << " lineto" << endl;
	m_f << xmax << " " << ymax << " lineto" << endl;
	m_f << xmax << " " << ymin << " lineto" << endl;
	m_f << xmin << " " << ymin << " lineto" << endl;

    m_f << "1 setlinewidth" << endl;
	m_f << a_lineGrayScale << " setgray" << endl;
	m_f << "stroke" << endl;
    m_f << "%%" << endl;
}

void PSDraw::solidbox(float a_xmin, float a_ymin,
                      float a_xmax, float a_ymax,
                      float a_lineGrayScale)
{
	int xmin = (int)ceil((a_xmin - m_cwminx) * m_xscale + m_minx);
	int ymin = (int)ceil((a_ymin - m_cwminy) * m_yscale + m_miny);
	int xmax = (int)ceil((a_xmax - m_cwminx) * m_xscale + m_minx);
	int ymax = (int)ceil((a_ymax - m_cwminy) * m_yscale + m_miny);

    //m_f << "%% draw box covering (";
    //m_f << a_xmin << "," << a_ymin << "),(";
    //m_f << a_xmax << "," << a_ymax << ")" << endl;
	m_f << "newpath" << endl;
	m_f << xmin << " " << ymin << " moveto" << endl;
	m_f << xmin << " " << ymax << " lineto" << endl;
	m_f << xmax << " " << ymax << " lineto" << endl;
	m_f << xmax << " " << ymin << " lineto" << endl;
	m_f << xmin << " " << ymin << " lineto" << endl;

    m_f << "1 setlinewidth" << endl;
	m_f << a_lineGrayScale << " setgray" << endl;
	m_f << "fill" << endl;
    m_f << "stroke" << endl;
    m_f << "%%" << endl;
}

void PSDraw::polygon(float* a_x, float* a_y, const int a_n,
                     float a_lineGrayScale)
{
    m_f << "%% draw polygon " << endl;
	m_f << "newpath\n" << endl;

	int x = (int)ceil((a_x[a_n-1]-m_cwminx) * m_xscale + m_miny);
	int y = (int)ceil((a_y[a_n-1]-m_cwminy) * m_yscale + m_minx);
	m_f << x << " " << y << " moveto" << endl;
	for (int i=0; i<a_n; i++)
	{
	    int x = (int)ceil((a_x[i]-m_cwminx) * m_xscale + m_miny);
	    int y = (int)ceil((a_y[i]-m_cwminy) * m_yscale + m_minx);
		m_f << x << " " << y << " lineto" << endl;
	}

	m_f << "1 setlinewidth" << endl;
	m_f << a_lineGrayScale << " setgray" << endl;
	m_f << "stroke" << endl;
    m_f << "%%" << endl;
}

void PSDraw::line(float a_xstart, float a_ystart,
                  float a_xend, float a_yend,
                  float a_red, float a_green, float a_blue,
                  int width)
{
    int xstart = (int)ceil((a_xstart - m_cwminx) * m_xscale + m_minx);
    int ystart = (int)ceil((a_ystart - m_cwminy) * m_yscale + m_miny);
    int xend   = (int)ceil((a_xend - m_cwminx) * m_xscale + m_minx);
    int yend   = (int)ceil((a_yend - m_cwminy) * m_yscale + m_miny);

    int xdiff = xend - xstart;
    int ydiff = yend - ystart;
    xdiff = xdiff > 0 ? xdiff : -xdiff;
    ydiff = ydiff > 0 ? ydiff : -ydiff;

    if (xdiff <= 0 && ydiff <= 0)
    {
        //point(a_xstart,ystart, a_red, a_green, a_blue);
        return;
    }
    

    //m_f << "%% draw line (";
    //m_f << xstart << "," << ystart << "),(";
    //m_f << xend << "," << yend << ")" << endl;
	m_f << "newpath" << endl;
	m_f << xstart << " " << ystart << " moveto" << endl;
	m_f << xend << " " << yend << " lineto" << endl;

    m_f << width << " setlinewidth" << endl;
	m_f << a_red << " " << a_green << " " << a_blue << " setrgbcolor" << endl;
	//m_f << a_lineGrayScale << " setgray" << endl;
	m_f << "stroke" << endl;
    m_f << "%%" << endl;
}

void PSDraw::box(float a_xmin, float a_ymin,
                 float a_xmax, float a_ymax,
                 float a_red, float a_green, float a_blue)
{
	int xmin = (int)ceil((a_xmin - m_cwminx) * m_xscale + m_minx);
	int ymin = (int)ceil((a_ymin - m_cwminy) * m_yscale + m_miny);
	int xmax = (int)ceil((a_xmax - m_cwminx) * m_xscale + m_minx);
	int ymax = (int)ceil((a_ymax - m_cwminy) * m_yscale + m_miny);

    //m_f << "%% draw box covering (";
    //m_f << a_xmin << "," << a_ymin << "),(";
    //m_f << a_xmax << "," << a_ymax << ")" << endl;
	m_f << "newpath" << endl;
	m_f << xmin << " " << ymin << " moveto" << endl;
	m_f << xmin << " " << ymax << " lineto" << endl;
	m_f << xmax << " " << ymax << " lineto" << endl;
	m_f << xmax << " " << ymin << " lineto" << endl;
	m_f << xmin << " " << ymin << " lineto" << endl;

    m_f << "1 setlinewidth" << endl;
	m_f << a_red << " " << a_green << " " << a_blue << " setrgbcolor" << endl;
    m_f << "fill" << endl;
	m_f << "stroke" << endl;
    m_f << "%%" << endl;
}


