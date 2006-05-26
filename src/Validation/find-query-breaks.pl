#!/usr/bin/perl -w
use strict;

use AMOS::AmosFoundation;

my $TRIMWIGGLE = 10;
my $CIRCLEWIGGLE = 0;
my $MINLENGTH = 0;
my $FLAGDIST = undef;
my $FLAGSAME = undef;
my $DOAMOS = 0;

my $HELPTEXT = qq~
Find alignment breaks in query sequences.

Note: You should probably run 'delta-filter -q out.delta > out.delta.q'
      and then check the out.delta.q file.

  find-query-breaks.pl [options] deltafile

  -b <val> Minimum length of alignment break to report (Default: 10)
  -c <val> Minimum distance to edge of reference sequence to report (Default: 0)
  -l <val> Minimum length of query sequence to report (Default: 0)
  -f <val> Flag broken alignments within this distance of reference
  -s       Flag adjacent broken alignments from same query

  -a       Display breaks as AMOS features
~;


sub printAlignment
{
  my $align = shift;
  my $rc = ($align->{qrc}) ? "[rc]" : "[]";
  print "$align->{rid} {1 [$align->{rstart},$align->{rend}] $align->{rlen}} | $align->{qid} {1 [$align->{qstart},$align->{qend}] $align->{qlen}} $rc";
}

my $base = new AMOS::AmosFoundation;
$base->setHelpText($HELPTEXT);
$base->setUsage("find-query-breaks.pl [options] deltafile");

my $err = $base->getOptions("b=i" => \$TRIMWIGGLE,
                            "c=i" => \$CIRCLEWIGGLE,
                            "l=i" => \$MINLENGTH,
                            "f=i" => \$FLAGDIST,
                            "s"   => \$FLAGSAME,
                            "a"   => \$DOAMOS);

if (!$err) { $base->bail("Command line parsing failed. See -h option"); }

if (scalar @ARGV == 0)
{
  print "Usage: ".$base->getUsage()."\n";
  exit(0);
}

my $multidelta = (scalar @ARGV) > 1;

foreach my $deltafile (@ARGV)
{
  print ">$deltafile\n" if $multidelta;

  my $cmd = "show-coords -Hcrl $deltafile";
  open COORDS, "$cmd |" or die "Can't run $cmd ($!)\n";

  my $lastalign = undef;

  while (<COORDS>)
  {
    #print $_;

    my @vals = split /\s+/, $_;

    my $align;
    $align->{rstart} = $vals[1];
    $align->{rend}   = $vals[2];

    $align->{qstart} = $vals[4];
    $align->{qend}   = $vals[5];

    $align->{ralen}   = $vals[7];
    $align->{qalen}   = $vals[8];

    $align->{pid}    = $vals[10];

    $align->{rlen}   = $vals[12];
    $align->{qlen}   = $vals[13];

    $align->{rid}    = $vals[18];
    $align->{qid}    = $vals[19];

    $align->{qrc}    = ($align->{qend} < $align->{qstart}) ? 1 : 0;

    if ($align->{qrc})
    {
      my $t = $align->{qstart};
      $align->{qstart} = $align->{qend};
      $align->{qend} = $t;
    }

    next if ($align->{qlen} < $MINLENGTH);

    if (defined $lastalign)
    {
      my $dist = $align->{rstart} - $lastalign->{rend};
 #     print "lid: $lastalign->{rid} id: $align->{rid} dist: $dist\n"
    }

    my $flag = "";
    if ((defined $FLAGDIST) &&
        (defined $lastalign) &&
        ($lastalign->{rid} eq $align->{rid}) &&
        ($align->{rstart} - $lastalign->{rend} < $FLAGDIST))
    {
      $flag = "***";
    }

    if ((defined $lastalign) &&
        (defined $FLAGSAME) &&
        ($lastalign->{rid} eq $align->{rid}) &&
        ($lastalign->{qid} eq $align->{qid})) 
    {
      $flag = "***";
    }
        

    my $breakcount = 0;


    if (( $align->{qrc} && ($align->{rlen} - $align->{rend} <= $CIRCLEWIGGLE)) ||
        (!$align->{qrc} && ($align->{rstart} - 1            <= $CIRCLEWIGGLE)))

    {

    }
    elsif (($align->{qstart} - 1) > $TRIMWIGGLE)
    {
      my $dist = $align->{qstart} - 1;
      my $len = $align->{qend} - $align->{qstart};
      $breakcount++;

      if ($DOAMOS)
      {
        my $s = $align->{qstart};
        my $e = $s+1;
        
        print "$align->{qid}\tB\t$e\t$s\tSTART_BREAK: $dist ($len)\n";
      }
      else
      {

        print "S-Break: $dist\tAlen: $len $flag\t";
        printAlignment($align);
        print "\n";
      }
    }

    if ((!$align->{qrc} && ($align->{rlen} - $align->{rend} <= $CIRCLEWIGGLE)) ||
        ( $align->{qrc} && ($align->{rstart} - 1            <= $CIRCLEWIGGLE)))

    {

    }
    elsif (($align->{qlen} - $align->{qend}) > $TRIMWIGGLE)
    {
      my $dist = $align->{qlen} - $align->{qend};
      my $len = $align->{qend} - $align->{qstart};
      $breakcount++;

      if ($DOAMOS)
      {
        my $s = $align->{qend};
        my $e = $s+1;
        
        print "$align->{qid}\tB\t$s\t$e\tEND_BREAK: $dist ($len)\n";
      }
      else
      {
        print "E-Break: $dist\tAlen: $len $flag\t";
        printAlignment($align);
        print "\n";
      }
    }

    if ($breakcount)
    {
      $lastalign = $align;
    }

    #print $_ if $breakcount;
  }
}
