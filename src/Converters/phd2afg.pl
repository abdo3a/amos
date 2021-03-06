#!/usr/bin/perl

use AMOS::AmosFoundation;

my $base = new AMOS::AmosFoundation;

if (! defined $base) {
    die ("Foundation cannot be created. FATAL!\n");
}

my $HELPTEXT = q~
.USAGE.
 phd2afg -d phd_dir -o file.afg [-c file.clr] [-m file.mates]

.DESCRIPTION.
  This program takes a directory of phd files and produces a .afg file
  with options for clear ranges and mates.

.OPTIONS.
  -d phd_dir     directory where phd files are located
  -o file.afg    name of the output file
  -c file.clr    optional: file containing clipping coordinates
  -m file.mates  optional: file containing mate-pair info in Bambus .mates format

.KEYWORDS.
 converters, phd, afg 
    ~;

$base->setHelpText($HELPTEXT);


my $phddir = undef;
my $outfile = undef;
my $matefile = undef;
my $clrfile = undef;

my $err = $base->getOptions(
			    "d=s" => \$phddir,
			    "o=s" => \$outfile,
			    "m=s" => \$matefile,
			    "c=s" => \$clrfile
			    );

if (! $err){
    $base->bail("Error processing options !");
}

if (! defined $phddir || ! defined $outfile){
    $base->bail("Both options -d and -o must be specified (see -h)\n");
}

open(OUT, ">$outfile") || die ("Cannot open $outfile:$!\n");

my %libraries; # store library information
my %seqids;    # identifiers attached with each sequence
my %seqnames;  # names attached with each identifier
my %seqrev;    # which revision of the sequence we are dealing with
my %seqfile;   # phd file corresponding to each sequence
my %insertlib; # mapping from libraries to inserts
my %seenlib;   # mapping from insert to library
my %insid;     # mapping from insert name to id
my %forw;      # forward read in each insert
my %rev;       # reverse read in each insert
my %seqinsert; # insert for each sequence
my %clears;    # clear ranges for the reads

$minSeqID = 1;

if (defined $clrfile){
    open(CLR, $clrfile) || die ("Cannot open $clrfile: $!\n");
    while (<CLR>){
	chomp;
	my ($id, $l, $r) = split(' ', $_);
#	$id =~ s/\.ab1$//;
	$clears{$id} = "$l,$r";
    }
    close(CLR);
}

opendir(PHD, $phddir) || die ("Cannot open $phddir: $!\n");

# get all names & associate name to latest phd file
# parse mates file
# start writing .afg file:
#   first libraries
#   then fragments
#   then reads: open each phd file and produce the corresponding afg record

while (my $file = readdir(PHD)){
    if ($file =~ /^\./){next;} # skip hidden files
    if ($file !~ /\.phd\.\d/) {next;} # skip non-phd files
    $file =~ /^(.*)\.phd\.(\d)/;
    my $seqname = $1;
    my $rev = $3;

    if (defined $clrfile && ! exists $clears{$seqname}){
	print "No clear range for $seqname\n";
	next;
    } # skip reads with no clear range

    if (! exists $seqfile{$seqname} ||
	$seqrev{$seqname} < $rev){
	$seqfile{$seqname} = "$phddir/$file";
	$seqrev{$seqname} = $rev;
    }
    if (! exists $seqids{$seqname}){
	$seqids{$seqname} = $minSeqID++;
	$seqnames{$seqids{$seqname}} = $seqname;
    }
}

closedir(PHD);

if (defined $matefile){
    open(MATE, $matefile) || die ("Cannot open $matefile: $!\n");
#    print STDERR "Parsing $matefile\n";
    parseMatesFile(\*MATE);
    close(MATE);
}

my $ll = $minSeqID++;
$libraries{$ll} = "0 0";
$libnames{$ll} = "dummy";
$libid{$ll} = $ll;

my $needdummy = 0;
while (my ($sid, $sname) = each %seqnames){
    if (! exists $seqinsert{$sid}){
	my $id = $minSeqID++;
	$seqinsert{$sid} = $id;
#	$insid{$id} = $id;
	$seenlib{$id} = $ll;
	$insertlib{$ll} .= "$id ";
	$forw{$id} = $sid;
	$needdummy = 1;
    }
}

if ($needdummy == 0){
    delete $libraries{$ll};
    delete $libnames{$ll};
    delete $libid{$ll};
}

my $date = localtime();

print OUT "{UNV\n";
print OUT "eid:afg\n";
print OUT "com:\n";
print OUT "generated by $0\n";
print OUT "on: $date\n";
print OUT ".\n";
print OUT "}\n";

# then print out one library at a time
while (my ($lib, $range) = each %libraries){
    my ($mean, $sd) = split(' ', $range);
    print OUT "{LIB\n";

    $libid{$lib} = $minSeqID;
    print OUT "iid:", $minSeqID++, "\n";

    if (exists $libnames{$lib}){
        print OUT "eid:$libnames{$lib}\n";
    } else {
        print OUT "eid:$lib\n";
    }
    print OUT "{DST\n";
    print OUT "mea:$mean\n";
    print OUT "std:", sprintf("%.2f\n", $sd);
    print OUT "}\n";
    print OUT "}\n";
}

# then all the inserts
while (my ($ins, $lib) = each %seenlib){
    if (! exists $insid{$ins}) {
	$insid{$ins} = $minSeqID++;
    };
    print OUT "{FRG\n";
    print OUT "eid:$ins\n";
    print OUT "iid:$insid{$ins}\n";
    print OUT "lib:$libid{$lib}\n";

# add rds.  
    if (exists $forw{$ins} && exists $rev{$ins}){
	print OUT "rds:$forw{$ins},$rev{$ins}\n";
    }

    print OUT "typ:I\n";
    print OUT "}\n";
}

# then all the sequences
while (my ($sid, $sname) = each %seqnames){
    my $seq;
    my $qual;
    my $pos;
    my $cll;
    my $clr;

    open(SEQ, $seqfile{$sname}) || die ("Cannot open $seqfile{$sname}: $!\n");
    parsePHDFile(\*SEQ, \$seq, \$qual, \$pos, \$cll, \$clr);
    close(SEQ);

    if (defined $clrfile){
	if (! exists $clears{$sname}){
	    die ("Bizarre.... should have clear range for $sname\n");
	}
	($cll, $clr) = split(',', $clears{$sname});
    }

    if ($cll < 0 || $clr < 0){next;} # skip bad sequences

    print OUT "{RED\n";
    print OUT "iid:$sid\n";
    print OUT "eid:$sname\n";
    print OUT "seq:\n";
    for (my $i = 0; $i < length($seq); $i+=60){
	print OUT substr($seq, $i, 60), "\n";
    }
    print OUT ".\n";
    print OUT "qlt:\n";
    for (my $i = 0; $i < length($qual); $i+=60){
	print OUT substr($qual, $i, 60), "\n";
    }
    print OUT ".\n";
    print OUT "bcp:\n";
    my @posn = split(' ', $pos);
    for (my $i = 0; $i <= $#posn; $i+=15){
	if ($i + 14 <= $#posn){
	    print OUT join(" ", @posn[$i .. $i+14]), "\n";
	} else {
	    print OUT join(" ", @posn[$i .. $#posn]), "\n";
	}
    }
    print OUT ".\n";
    print OUT "clr:$cll,$clr\n";
    if (! exists $seqinsert{$sid}){
	die ("Cannot find insert for $sid $sname\n");
    }
    print OUT "frg:$insid{$seqinsert{$sid}}\n";
    print OUT "}\n";
}

close(OUT);
exit(0);



####################################################

# parses a PHD file into 5 values;
# sequence, quality, positions, clear left, clear right
sub parsePHDFile()
{
    my $file = shift;
    my $seq = shift;
    my $qual = shift;
    my $pos = shift;
    my $cll = shift;
    my $clr = shift;
    
    $$cll = undef; $$clr = undef;
    $$seq = ""; $$qual = ""; $$pos = "";

    my @posn;
    
    my $inseq = 0;
    
    while (<$file>){
	if (/^BEGIN_DNA/){
	    $inseq = 1;
	    next;
	}
	if (/^END_DNA/){
	    $inseq = 0;
	    next;
	}
	if (/^TRIM: (\d+) (\d+) /){
	    $$cll = $1 - 1; $$clr = $2;
	    next;
	}
	if ($inseq == 1){
	    chomp;
	    my ($base, $q, $p) = split(" ", $_);
	    $$seq .= $base;
	    if ($q > 60) {$q = 60;}
	    $$qual .= chr(ord('0') + $q);
#	    if ($#posn == -1){
	    push @posn, $p;
#	    } else {
#		push @posn, $p; # - $posn[$#posn]; # delta encoding
#	    }
	}
    }

    $$pos = join(" ", @posn);
    
    if (! defined $$cll){
	$$cll = 0;
	$$clr = length($$seq);
    }

    return;
} # parsePHDFile

# parses BAMBUS style .mates file
# * expects %seqids to be populated
# * populates %libraries, %forw, %rev, %insertlib, %seenlib, %seqinsert
sub parseMatesFile {
    my $IN = shift;

    my @libregexp;
    my @libids;
    my @pairregexp;
    my $insname = 1;

    while (<$IN>){
	chomp;
	if (/^library/){
	    my @recs = split('\t', $_);
	    if ($#recs < 3 || $#recs > 4){
		print STDERR "Only ", $#recs + 1, " fields\n";
		print STDERR "Improperly formated line $. in \"$matesfile\".\nMaybe you didn't use TABs to separate fields\n";
		next;
	    }
	    
	    if ($#recs == 4){
		$libregexp[++$#libregexp] = $recs[4];
		$libids[++$#libids] = $recs[1];
	    }
	    my $mean = ($recs[2] + $recs[3]) / 2;
	    my $stdev = ($recs[3] - $recs[2]) / 6;
	    $libraries{$recs[1]} = "$mean $stdev";
	    next;
	} # if library

	if (/^pair/){
	    my @recs = split('\t', $_);
	    if ($#recs != 2){
		print STDERR "Improperly formated line $. in \"$matesfile\".\nMaybe you didn't use TABs to separate fields\n";
		next;
	    }
	    @pairregexp[++$#pairregexp] = "$recs[1] $recs[2]";
	    next;
	}
	if (/^\#/) { # comment
	    next;
	}
	if (/^\s*$/) { # empty line
	    next;
	}
	
	# now we just deal with the pair lines
	my @recs = split('\t', $_);
	if ($#recs < 1 || $#recs > 2){
	    print STDERR "Improperly formated line $. in \"$matesfile\".\nMaybe you didn't use TABs to separate fields\n";
	    next;
	}
	
# make sure we've seen these sequences
	if (! defined $seqids{$recs[0]}){
	    print STDERR "Sequence $recs[0] has no ID at line $. in \"$matesfile\"\n";
	    next;
	}
	if (! defined $seqids{$recs[1]} ){
	    print STDERR "Sequence $recs[1] has no ID at line $. in \"$matesfile\"\n";
	    next;
	}
	
	if (defined $recs[2]){
	    $insertlib{$recs[2]} .= "$insname ";
	    $seenlib{$insname} = $recs[2];
	} else {
	    print STDERR "$insname has no library\n";
	}
	
	$forw{$insname} = $seqids{$recs[0]};
	$rev{$insname} = $seqids{$recs[1]};
	
	$seqinsert{$seqids{$recs[0]}} = $insname;
	$seqinsert{$seqids{$recs[1]}} = $insname;
	
	$insname++;
    } # while <IN>

    # now we have to go through all the sequences and assign them to
    # inserts
    while (my ($nm, $sid) = each %seqids){
	for (my $r = 0; $r <= $#pairregexp; $r++){
	    my ($freg, $revreg) = split(' ', $pairregexp[$r]);
	    print STDERR "trying $freg and $revreg on $nm\n";
	    if ($nm =~ /$freg/){
		my $iname = "";
		for (my $in = 1; $in <= $#+; $in++){
		    $iname .= substr($nm, $-[$in], $+[$in] - $-[$in]);
		}
		print STDERR "got forw $iname\n";
#imhere
		if (! exists $forw{$iname}){
		    $forw{$iname} = $sid;
		    $seqinsert{$sid} = $iname;
		}
		last;
	    }
	    if ($nm =~ /$revreg/){
		my $iname = "";
		for (my $in = 1; $in <= $#+; $in++){
		    $iname .= substr($nm, $-[$in], $+[$in] - $-[$in]);
		}
		print STDERR "got rev $iname\n";
		if (! exists $rev{$iname}){
		    $rev{$iname} = $sid;
		    $seqinsert{$sid} = $iname;
		}
		last;
	    }
	} # for each pairreg
    } # while each %seqids
    
    while (my ($ins, $nm) = each %forw) {
	if (! exists $seenlib{$ins}){
	    my $found = 0;
	    
	    $nm = $seqnames{$nm};

	    for (my $l = 0; $l <= $#libregexp; $l++){
		print STDERR "Trying $libregexp[$l] on $nm\n";
		if ($nm =~ /$libregexp[$l]/){
		    print STDERR "found $libids[$l]\n";
		    $insertlib{$libids[$l]} .= "$ins ";
		    $seenlib{$ins} = $libids[$l];
		    $found = 1;
		    last;
		}
	    }
	    if ($found == 0){
		print STDERR "Cannot find library for \"$nm\"\n";
		next;
	    }
	}
    }
    while (my ($ins, $nm) = each %rev) {
	if (! exists $seenlib{$ins}){
	    my $found = 0;
	    
	    $nm = $seqnames{$nm};

	    for (my $l = 0; $l <= $#libregexp; $l++){
		print STDERR "Trying $libregexp[$l] on $nm\n";
		if ($nm =~ /$libregexp[$l]/){
		    print STDERR "found $libids[$l]\n";
		    $insertlib{$libids[$l]} .= "$ins ";
		    $seenlib{$ins} = $libids[$l];
		    $found = 1;
		    last;
		}
	    }
	    if ($found == 0){
		print STDERR "Cannot find library for \"$nm\"\n";
		next;
	    }
	}
    }
} # parseMateFile;
