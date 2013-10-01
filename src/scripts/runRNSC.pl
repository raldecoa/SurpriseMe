#!/usr/bin/perl

use strict;
use warnings;

my $graph = $ARGV[0];  # Edge list


die "No input file specified!" unless @ARGV;
open( G, $graph ) or die "File $graph not found!";
close G;


system("./src/RNSC/rnscconvert -g a.graph -i $graph -n a.functions > /dev/null");
system("./src/RNSC/rnsc -g a.graph > /dev/null");
system("./src/RNSC/rnscfilter -g a.graph -c out.rnsc -n a.functions > /dev/null");

 
open( O, "output.txt" ) or die "File output.txt not found!";
my @lines = <O>;
close O;

my %part;
while( scalar @lines > 0 ){
    my $index = shift @lines;
    my $line = shift @lines;
    shift @lines;

    $index =~ /Cluster #(\d+)/;
    my $i = $1;
    $line =~ s/\s//g;
    my @nodes = split '\(U\)', $line;
    foreach(@nodes){
	$part{$_} = $i;
    }
}

my $part = $graph;
$part =~ s/\.\w+$/\_RNSC.part/;
open O, ">$part";
print O "RNSC\n";
foreach(sort keys %part){
    print O "$_\t".$part{$_}."\n";
}

close O;
system("rm a.functions a.graph out.rnsc output.txt");
