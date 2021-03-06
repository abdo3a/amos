#!/usr/bin/perl -w

use AMOS::AmosLib;
use TIGR::Foundation;

use strict;

my $VERSION = '$Revision$ ';
my $HELP = q~
.USAGE.
  amos2frg [-i infile] [-o outfile] [-a accession]

.DESCRIPTION.
  This program converts from an AMOS message file to a frg file (Celera
  Assembler message file).

.OPTIONS.
  if -i and -o are not provided reads from STDIN and writes to STDOUT
  if -i is provided but -o is not, outfile is same as infile except for the
  extension
  otherwise -i and -o are those specified in the command line
  if -i is provided the filename must end in .afg
  if -a is specified starts numbering output at specified accession, else
  the accessions from the input file are used in the output

.KEYWORDS.
  converter, amos, frg

~;

my $base = new TIGR::Foundation();
if (! defined $base) {
    die("A horrible death\n");
}


$base->setVersionInfo($VERSION);
$base->setHelpInfo($HELP);

my $infile;
my $outfile;
my $inname = "stdin";
my $accession = 1;
my $date = time();
my $firstunv = 1;
my %libids;
my %frg2lib;
my %rd2lib;
my %rd2typ;
my %rdids;
my %mates;

my $err = $base->TIGR_GetOptions("i=s"   => \$infile,
				 "o=s"   => \$outfile,
				 "a=i"   => \$accession);



# if infile is provided, make sure it ends in .afg and open it
if (defined $infile){
    if ($infile !~ /\.afg$/){
	$base->bail ("Input file name must end in .afg");
    }
    $inname = $infile;
    open(STDIN, "$infile") || $base->bail("Cannot open $infile: $!\n");
}

# if infile is provided but outfile isn't make outfile by changing the extension
if (! defined $outfile && defined $infile){
    $outfile = $infile;
    $outfile =~ s/\.afg$/.frg/;
}

# if outfile is provided (or computed above) simply open it
if (defined $outfile){
    open(STDOUT, ">$outfile") || $base->bail("Cannot open $outfile: $!\n");
}

while (my $record = getRecord(\*STDIN)){
    my ($type, $fields, $recs) = parseRecord($record);
    my $acc;

    $acc = $accession++;

#    print "Got $type\n";
    if ($type eq "UNV"){
	next unless $firstunv;
	
	print qq~{BAT
bna:CeleraAssembler
crt:$date
acc:$acc
com:
.
}
{ADT
{ADL
who:
ctm:$date
vsn:1.00
com:
Generated by $0 from file: $inname
.
}
{ADL
who:
ctm:$date
vsn:1.00
com:
AMOS UNV header
$$fields{com}.
}
.
}
~;

        $firstunv = 0;
    } # type is UNV

    if ($type eq "LIB"){
	# only use the first DST record  
	if ($#$recs < 0){
	    $base->bail("LIB record doesn't have any DST record at or around line $. in input");
	}
	$libids{$$fields{"iid"}} = $acc;
        my ($sid, $sfs, $srecs) = parseRecord($$recs[0]);
	if ($sid ne "DST"){
	    $base->bail("LIB record doesn't start with DST record at or around line $. in input");
	}
        if ($$sfs{mea} == 0) {
          $$sfs{mea} = 10;
        }
        if ($$sfs{std} == 0) {
          $$sfs{std} = 3;
        }
print qq~{DST
act:A
acc:$acc
mea:$$sfs{mea}
std:$$sfs{std}
}
~;
    } # type is LIB
    
    if ($type eq "FRG"){
	$frg2lib{$$fields{"iid"}} = $$fields{"lib"};
        if ( exists $$fields{"rds"} ) {
            $$fields{"rds"} =~ /^(\d+),(\d+)/;
            $mates{$1} = $2;
            $rd2typ{$1} = $$fields{"typ"};
        }
    } # type is FRG

    if ($type eq "RED"){
	$rd2lib{$$fields{"iid"}} = $frg2lib{$$fields{"frg"}};
	$rdids{$$fields{"iid"}} = $acc;

my @clr = split /,/, $$fields{clr};
if ($clr[0] > $clr[1])
{
  my $t = $clr[0];
  $clr[0] = $clr[1];
  $clr[1] = $t;
  $$fields{clr} = "$clr[0],$clr[1]";
}


print qq~{FRG
act:A
acc:$acc
typ:R
src:
$$fields{eid}
.
etm:0
seq:
$$fields{seq}.
qlt:
$$fields{qlt}.
clr:$$fields{clr}
}
~;

    } # type is RED
} # while each record

foreach my $rd1 ( keys %mates ) {
   my $rd2 = $mates{$rd1};
   if ($rd2lib{$rd1} != $rd2lib{$rd2}){
       $base->bail("Reads $rd1 and $rd2 don't appear to map to the same library ($rd2lib{$rd1} != $rd2lib{$rd2})");
   }

my $typ = $rd2typ{$rd1};
my $ori = "I";
if ($typ eq "T") 
{ 
  $ori = "O"; 
}
else
{
  $typ = "M";
}

print qq~{LKG
act:A
typ:$typ
fg1:$rdids{$rd1}
fg2:$rdids{$rd2}
etm:0
dst:$libids{$rd2lib{$rd1}}
ori:$ori
}
~;
} # for each mate

exit(0);
