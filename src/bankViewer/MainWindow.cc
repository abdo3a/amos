#include "MainWindow.hh"

#include <qapplication.h>
#include <qpushbutton.h>
#include <qfont.h>
#include <qlayout.h>
#include <qspinbox.h>
#include <qmenubar.h>
#include <qpopupmenu.h>
#include <qfiledialog.h>
#include <qstatusbar.h>
#include <qlabel.h>
#include <qscrollview.h>
#include <qlineedit.h>
#include <qcheckbox.h>
#include <qpushbutton.h>

#include <qmainwindow.h>
#include <qlistview.h>
#include <qcursor.h>

#include "TilingFrame.hh"

using namespace std;

MainWindow::MainWindow( QWidget *parent, const char *name )
           : QWidget( parent, name )
{
  // Menubar
  QMenuBar* menubar = new QMenuBar(this);
  Q_CHECK_PTR( menubar );

  m_contigPicker = NULL;

  QPopupMenu* file = new QPopupMenu( menubar );
  Q_CHECK_PTR( file );
  menubar->insertItem( "&File", file );
  file->insertItem("&Open Bank...", this,  SLOT(chooseBank()) );
  file->insertItem("&Contig Picker...", this, SLOT(chooseContig()));
  file->insertItem("&Quit", qApp,  SLOT(quit()), CTRL+Key_Q );

  // Statusbar
  QLabel * statusbar = new QLabel(this);
  statusbar->setFrameStyle(QFrame::Panel | QFrame::Sunken);
  statusbar->setText("No Bank Loaded");

  // Widgets
  m_contigid = new QSpinBox(1, 1, 1, this, "contigid");
  m_gindex   = new QSpinBox(0,100, 1, this, "gindexspin");

  QSpinBox * fontsize  = new QSpinBox(6, 24, 1, this, "fontsize");
  QLineEdit * dbpick   = new QLineEdit("DMG", this, "dbpick");
  TilingFrame * tiling = new TilingFrame(this, "tilingframe");
  QPushButton * showinserts = new QPushButton("Show Inserts", this, "showinserts");

  QCheckBox * stable       = new QCheckBox("Stable Tiling", this, "stable");
  QCheckBox * shownumbers  = new QCheckBox("Show Position", this, "consnumbers");
  QCheckBox * hldisc       = new QCheckBox("Highlight", this, "highlightconflicts");
  QCheckBox * showqv       = new QCheckBox("Show QV", this, "showqvchkbx");

  QLabel * contig_lbl   = new QLabel(m_contigid, "Contig ID", this, "contiglbl");
  QLabel * db_lbl       = new QLabel(dbpick, "Database", this, "dblbl");
  QLabel * gindex_lbl   = new QLabel(m_gindex, "Position", this, "gindexlbl");
  QLabel * fontsize_lbl = new QLabel(fontsize, "Font Size", this, "fontlbl");

  int gutter = 5;

  m_slider = new QScrollBar(Horizontal, this, "slider");
  m_slider->setTracking(0);
  m_slider->setPageStep(20);

  // slider <-> tiling
  connect(m_slider, SIGNAL(valueChanged(int)),
          tiling, SLOT(setGindex(int)) );

  connect(m_slider, SIGNAL(sliderMoved(int)),
          tiling, SLOT(trackGindex(int)));

  connect(tiling, SIGNAL(gindexChanged(int)),
          m_slider, SLOT(setValue(int)) );

  connect(m_slider, SIGNAL(sliderReleased()),
          tiling,   SLOT(trackGindexDone()) );

  // m_gindex <-> tiling
  connect(tiling, SIGNAL(gindexChanged(int)),
          m_gindex, SLOT(setValue(int)));

  connect(m_gindex, SIGNAL(valueChanged(int)),
          tiling, SLOT(setGindex(int)));

  // fontsize <-> tiling
  connect(fontsize, SIGNAL(valueChanged(int)), 
          tiling,   SLOT(setFontSize(int)));

  // checkboxes <-> tiling
  connect(stable, SIGNAL(toggled(bool)),
          tiling, SLOT(toggleStable(bool)));
  connect(shownumbers, SIGNAL(toggled(bool)),
          tiling, SLOT(toggleShowNumbers(bool)));
  connect(hldisc, SIGNAL(toggled(bool)),
          tiling, SLOT(toggleHighlightDiscrepancy(bool)));
  connect(showqv, SIGNAL(toggled(bool)),
          tiling, SLOT(toggleDisplayQV(bool)));

  connect(showinserts, SIGNAL(clicked()),
          tiling,      SLOT(showInserts()));


  // contigid <-> tiling
  connect(m_contigid, SIGNAL(valueChanged(int)),
          tiling,     SLOT(setContigId(int)));

  connect(tiling,     SIGNAL(contigLoaded(int)),
          m_contigid, SLOT(setValue(int)));

  // mainwindow <-> tiling
  connect(this,   SIGNAL(bankSelected(std::string)),
          tiling, SLOT(setBankname(std::string)));
  
  connect(tiling,   SIGNAL(contigRange(int, int)),
          this,     SLOT(setContigRange(int, int)));

  connect(tiling, SIGNAL(setGindexRange(int, int)),
          this,   SLOT(setGindexRange(int, int)));

  connect(this, SIGNAL(contigIdSelected(int)),
          tiling, SLOT(setContigId(int)));

  connect(this, SIGNAL(gindexChanged(int)),
          tiling, SLOT(setGindex(int)));

  // statusbar <-> tiling
  connect(tiling,    SIGNAL(setStatus(const QString &)),
          statusbar, SLOT(setText(const QString &)));

  // dbpick <-> tiling
  connect(dbpick, SIGNAL(textChanged(const QString &)),
          tiling, SLOT(setDB(const QString &)));

  
  // outmost
  QBoxLayout * outergrid = new QHBoxLayout(0);
  QVBoxLayout* vbox = new QVBoxLayout(this);
  vbox->setMenuBar(menubar);
  menubar->setSeparator(QMenuBar::InWindowsStyle);
  vbox->addLayout(outergrid);
  vbox->addSpacing(gutter);
  vbox->addWidget(statusbar);
  vbox->activate();
  
  // outer
  QBoxLayout * leftgrid  = new QVBoxLayout(0);
  QBoxLayout * rightgrid = new QVBoxLayout(0);
 
  outergrid->addSpacing(gutter);
  outergrid->addLayout(leftgrid);
  outergrid->addSpacing(gutter);

  outergrid->addLayout(rightgrid, 10);

  //left
  leftgrid->addSpacing(gutter);
  leftgrid->addWidget(showinserts);
  leftgrid->addWidget(db_lbl);
  leftgrid->addWidget(dbpick);
  leftgrid->addSpacing(gutter);

  leftgrid->addWidget(contig_lbl);
  leftgrid->addWidget(m_contigid);
  leftgrid->addSpacing(gutter);

  leftgrid->addWidget(gindex_lbl);
  leftgrid->addWidget(m_gindex);
  leftgrid->addSpacing(gutter);

  leftgrid->addWidget(fontsize_lbl);
  leftgrid->addWidget(fontsize);
  leftgrid->addSpacing(gutter);

  leftgrid->addWidget(stable);
  leftgrid->addWidget(shownumbers);
  leftgrid->addWidget(hldisc);
  leftgrid->addWidget(showqv);
  leftgrid->addStretch(gutter);

  //right
  rightgrid->addWidget(tiling, gutter);
  rightgrid->addWidget(m_slider);

  // Set defaults
  fontsize->setValue(12);
  m_gindex->setValue(0);
  m_slider->setFocus();
}

void MainWindow::setBankname(std::string bankname)
{
  m_bankname = bankname;
  emit bankSelected(bankname);
}

void MainWindow::setGindex(int gindex)
{
  emit gindexChanged(gindex);
}

void MainWindow::setContigId(int contigId)
{
  emit contigIdSelected(contigId);
}

void MainWindow::chooseBank()
{
  QString s = QFileDialog::getExistingDirectory(
                   "/local/asmg/work/mschatz/AMOS",
                   this,
                   "get existing directory",
                   "Choose a Bank",
                   TRUE );

  if (s != "")
  {
    m_bankname = s.ascii();
    emit bankSelected(s.ascii());
  }
}

void MainWindow::setContigRange(int a, int b)
{
  m_contigid->setRange(a,b);
}

void MainWindow::setGindexRange(int a, int b)
{
  m_gindex->setRange(a,b);
  m_slider->setRange(a,b);
}

class ContigListItem : public QListViewItem
{
public:
  ContigListItem(QListView * parent, 
                 QString id,
                 QString type,
                 QString offset,
                 QString length,
                 QString elements)
               
    : QListViewItem(parent, id, type, offset, length, elements) {}

  ContigListItem(QListViewItem * parent, 
                 QString id,
                 QString type,
                 QString offset,
                 QString length,
                 QString elements)
               
    : QListViewItem(parent, id, type, offset, length, elements) {}

  int compare(QListViewItem *i, int col,
              bool ascending ) const
  {
    return atoi(key(col,ascending)) - atoi(i->key(col,ascending));
  }
};

void MainWindow::chooseContig()
{
  if (m_contigPicker) { delete m_contigPicker; }

  m_contigPicker = new QMainWindow(this);

  QListView * table = new QListView(m_contigPicker, "contigpicker");
  table->resize(500,500);
  m_contigPicker->setCentralWidget(table);
  m_contigPicker->resize(300,300);
  m_contigPicker->show();

  connect(table, SIGNAL(doubleClicked(QListViewItem *)),
          this,  SLOT(contigSelected(QListViewItem *)));

  table->addColumn("Id");
  table->addColumn("Type");
  table->addColumn("Offset");
  table->addColumn("Length");
  table->addColumn("Elements");
  table->setShowSortIndicator(true);
  table->setRootIsDecorated(true);

  try
  {
    QCursor orig = m_contigPicker->cursor();
    m_contigPicker->setCursor(Qt::waitCursor);
    AMOS::Contig_t contig;
    AMOS::BankStream_t contig_bank(AMOS::Contig_t::NCODE);

    contig_bank.open(m_bankname);

    QString status = "Select from " + 
                     QString::number(contig_bank.getSize()) + 
                     " contigs in " + m_bankname.c_str();
    m_contigPicker->statusBar()->message(status);

    vector<ContigListItem> contigs;

    int contigid = 1;
    while (contig_bank >> contig)
    {
      int contiglen = contig.getSeqString().length();
      int numreads = contig.getReadTiling().size();

      ContigListItem * contigitem = new ContigListItem(table,  
                                                       QString::number(contigid), 
                                                       "Contig",
                                                       "0",
                                                       QString::number(contiglen), 
                                                       QString::number(numreads));

      /*
      vector<AMOS::Tile_t>::iterator ti;
      for (ti =  contig.getReadTiling().begin();
           ti != contig.getReadTiling().end();
           ti++)
      {
        new ContigListItem(contigitem,
                           QString::number(ti->source),
                           "Read",
                           QString::number(ti->offset),
                           QString::number(ti->range.getLength() + ti->gaps.size()),
                           " ");
      }
      */
      
      contigid++;
    }
    m_contigPicker->setCursor(orig);

  }
  catch (AMOS::Exception_t & e)
  {
    cerr << "ERROR: -- Fatal AMOS Exception --\n" << e;
  }
}

void MainWindow::contigSelected(QListViewItem * item)
{
  emit contigIdSelected(atoi(item->text(0)));
  m_contigPicker->close();
  m_contigPicker = NULL;
}


