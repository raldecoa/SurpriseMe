#!/usr/bin/perl

# Copyright 2013 Rodrigo Aldecoa and Ignacio Marín
# Contact: raldecoa@ibv.csic.es

# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.

use strict;
use warnings;
use threads;

if(scalar @ARGV != 1){
    print "No network file specified!\n";
    print "Usage: ./SurpriseMe.pl network_file\n";
    exit();
} 

my $maxThreads = 3;
my @algs = ( "UVCluster", "RN", "CPM", "RB", "Infomap", "RNSC", "SCluster" );
my $f = $ARGV[0];
open F, $f or die "Network file $f not found!";
close F;

## Reduce number of threads for low CPUs
#if(scalar @lines > 10000){
#    $maxThreads = 1;
#}

# Run a thread for each algorithm
my @Threads;
my @running = ();
my $nTasks = scalar @algs;
my $index = 0;
while(scalar @Threads < $nTasks){
    @running = threads->list(threads::running);
    if(scalar @running < $maxThreads){
	my $thread = threads-> create( sub { runIt($algs[$index], $f ) });
	$index++;
	push @Threads, $thread;
    }
    sleep(1);
}
$_->join for @Threads;


# Compute Surprise for each solution
my %S;
foreach(@algs){
    my $p = $f;
    $p =~ s/\.\w+$/\_$_\.part/;
    system("./src/Surprise/computeSurprise $f $p > tmp");
    my $tmp = &getFromFile("tmp");
    system("rm tmp");
    $S{$_} = $tmp;
}

# Write ranking of solutions
my $out = $f;
$out =~ s/\.\w+$/\.S/;
open O, ">$out";
print O "Algorithm\tS\n";
print O "----------\t--\n";
foreach(sort{ $S{$b} <=> $S{$a} } keys %S){
    print O "$_\t".$S{$_}."\n";
}
close O;


# Create ONE and SINGLES partition
my %nodes;
open F, $f;
while(<F>){
    my ($a, $b) = split '\s', $_;
    $nodes{$a} = $nodes{$b} = 1;
}
close F;

my $name = $f;
$name =~ s/\.\w+$//;
open ONE, ">$name\_ONE.part";
open SINGLES, ">$name\_SINGLES.part";
print ONE "ONE\n";
print SINGLES "SINGLES\n";
$index = 0;
foreach(keys %nodes){
    print ONE "$_\t0\n";
    print SINGLES "$_\t$index\n";
    $index++;
}
close ONE;
close SINGLES;

# Add both partitions
push @algs, "ONE";
push @algs, "SINGLES";
$S{"ONE"} = $S{"SINGLES"} = 0;

# Compute VI between each pair of solutions
my (%matrixVI, %matrixNMI);
for(my $i = 0; $i < scalar @algs; $i++){
    my $a1 = $algs[$i];
    for(my $j = $i+1; $j < scalar @algs; $j++){
	my $a2 = $algs[$j];
	my @result = &calcVI($f, $a1, $a2);
	my ($vi, $nmi) = ($result[0], $result[1]);
	$matrixVI{$a1}{$a2} = $matrixVI{$a2}{$a1} = $vi;
	$matrixNMI{$a1}{$a2} = $matrixNMI{$a2}{$a1} = $nmi;
    }
}

# Write distance matrixVI to file
$out = $f;
$out =~ s/\.\w+$/\_VI.meg/;
open O, ">$out";
print O "#MEGA\n";
print O "!TITLE=$f;\n";
print O "!Format Datatype=distance Dataformat=lowerleft;\n";
print O "!Description;\n";
foreach(@algs){
    print O "#$_\_{".$S{$_}."}\n";
}

for(my $i = 1; $i < scalar @algs; $i++){
    my $a1 = $algs[$i];
    for(my $j = 0; $j < $i; $j++){
	my $a2 = $algs[$j];
	print O $matrixVI{$a1}{$a2};
	if($j != $i-1){
	    print O "\t";
	}
    }
    print O "\n";
}
close O;


# Write distance matrixVI to file
$out = $f;
$out =~ s/\.\w+$/\_1-NMI.meg/;
open O, ">$out";
print O "#MEGA\n";
print O "!TITLE=$f;\n";
print O "!Format Datatype=distance Dataformat=lowerleft;\n";
print O "!Description;\n";
foreach(@algs){
    print O "#$_\_{".$S{$_}."}\n";
}

for(my $i = 1; $i < scalar @algs; $i++){
    my $a1 = $algs[$i];
    for(my $j = 0; $j < $i; $j++){
	my $a2 = $algs[$j];
	print O $matrixNMI{$a1}{$a2};
	if($j != $i-1){
	    print O "\t";
	}
    }
    print O "\n";
}
close O;


sub runIt{
    my $name = shift @_;
    my $f = shift @_;
    system("./src/scripts/run$name\.pl $f");
}

sub calcVI{
    my $f = shift @_;
    my $a = shift @_;
    my $b = shift @_;
    $f =~ s/\.\w+$/\_/;
    my $fa = "$f$a.part";
    my $fb = "$f$b.part";
    system("./src/scripts/VI $fa $fb > tmp");
    open F, "tmp";
    my @lines = <F>;
    close F;
    system("rm tmp");
    my ($vi, $nmi);
    if($lines[0] =~ /(.*) - (.*)/){
	$vi = $1;
	$nmi = $2;
    }
    return ($vi, $nmi);
}

sub getFromFile{
    my $f = shift @_;
    open F, $f;
    my @lines = <F>;
    close F;
    chomp $lines[0];
    $lines[0] =~ s/Surprise = //;
    return $lines[0];
}
