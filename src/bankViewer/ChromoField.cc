#include "ChromoField.hh"
#include <qpainter.h>
#include <qpen.h>
#include <math.h>
#include <qstring.h>
#include <string>
#include <qpixmap.h>

#include <stdio.h>
#include "UIElements.hh"




ChromoField::ChromoField(RenderSeq_t * read, 
                         const string & db, 
                         const string & cons,
                         QWidget *parent, 
                         const char *name)
  :QWidget(parent, name)
{
  setPalette(QPalette(QColor(180, 180, 180)));

  string readname = read->m_read.getEID();

  string path = "/local/chromo/Chromatograms/";
  path += db + "/ABISSed/" + readname[0]+readname[1]+readname[2] + "/"
                           + readname[0]+readname[1]+readname[2]+readname[3] + "/"
                           + readname[0]+readname[1]+readname[2]+readname[3]+readname[4]+ "/" 
                           + readname;

  m_render = read;
  m_rawread = read_reading((char *)path.c_str(), TT_ANY);
  m_pix = NULL;

  if (!m_rawread)
  {
    return;
  }

  resize (m_rawread->NPoints+100,160);

  vector<int> pos;

  char name[100];
  int version;

  char hex[5];
  hex[4]='\0';
  string curseq;
  FILE * fpos = fopen("dmg.pos", "r");
  char c;
  int i;
  while ((i = fscanf ( fpos, "%s\t%d\t", name, &version )) != 0)
  {
    if (readname == name)
    {
      while ( fgets ( hex, 5, fpos ) && hex[0] != '\n' )
      {
          int x;

          sscanf ( hex, "%04x", &x );
          pos.push_back(x);
      }

      break;
    }
    else
    {
      do {
        c = fgetc(fpos);
      } while (c != '\n');
    }
  }

  cerr << "loaded " << pos.size() << "positions, i=" << i << endl;

  m_pix = new QPixmap(width(), height());
  m_pix->fill(this, 0, 0);

  QPen pen;
  QPainter painter(m_pix);

  int baseline = 100;
  int offset = 20;

  int vscale=24;
  int tickwidth = 5;
  int maxy = 2000;

  // axis
  painter.drawLine(offset,baseline,m_rawread->NPoints+offset,baseline);
  painter.drawLine(offset,baseline,offset,baseline-(maxy/vscale));

  // y-ticks
  for (i = 0; i < maxy; i+=100)
  {
    int y = baseline-(i/vscale);
    painter.drawLine(offset,y,offset+tickwidth,y);
  }

  // x-ticks
  for (i = 0; i < m_rawread->NPoints; i+=10)
  {
    painter.drawLine(i+offset,baseline,i+offset,baseline-tickwidth);
    if (!(i % 50))
    {
      painter.drawLine(i+offset, baseline+tickwidth, i+offset, baseline);
    }
  }

  // x-labels
  painter.setFont(QFont("Helvetica", 8));
  for (i=0; i < m_rawread->NPoints; i += 50)
  {
    QString s = QString::number(i);
    painter.drawText(i-20+offset,baseline+10,40,20,Qt::AlignHCenter,s);
  }
  painter.setFont(QFont("Helvetica", 12));

  for (int channel = 0; channel < 4; channel++)
  {
    unsigned short * trace = NULL;
    switch (channel)
    {
      case 0: trace = m_rawread->traceA; UIElements::setBasePen(pen, 'A'); break;
      case 1: trace = m_rawread->traceC; UIElements::setBasePen(pen, 'C'); break;
      case 2: trace = m_rawread->traceG; UIElements::setBasePen(pen, 'G'); break;
      case 3: trace = m_rawread->traceT; UIElements::setBasePen(pen, 'T'); break;
    };

    painter.setPen(pen);

    painter.moveTo(offset,baseline);
    for (i = 0; i < m_rawread->NPoints; i++)
    {
      if (trace[i])
      {
        painter.lineTo(i+offset,baseline-(trace[i])/vscale);
      }
      else
      {
        painter.moveTo(i+offset,baseline);
      }
    }
  }

  string bases = read->m_read.getSeqString();

  for (i = 0; i < pos.size(); i++)
  {
    char b = bases[i];
    UIElements::setBasePen(pen, b);
    painter.setPen(pen);

    QString s;
    s+= b;
    painter.drawText(pos[i]-20+offset,baseline+25,40,20,Qt::AlignHCenter,s);

    pen.setColor(black);
    painter.setPen(pen);

    int gindex = read->getGindex(i);
    if (gindex >= read->m_offset && gindex < read->m_offset + read->m_nucs.size())
    {
      s = "";
      s += cons[gindex];
      painter.drawText(pos[i]-20+offset,baseline+40,40,20,Qt::AlignHCenter,s);
    }
  }

  painter.end();
}

void ChromoField::paintEvent(QPaintEvent * event)
{
  if (m_pix)
  {
    QPainter painter(this);
    painter.drawPixmap(0, 0, *m_pix);
  }
}

