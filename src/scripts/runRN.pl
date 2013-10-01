#!/usr/bin/perl

use strict;
use warnings;

my $file = $ARGV[0];
open(F, $file) or die "Wrong input file!";
close F;

my $inputFile = $file;

# .pairs to .gml
system("./src/scripts/txt2gml.pl $file");
$file =~ s/\.\w+$/.gml/;

open(F, $file);
my @lines = <F>; 
close F;

my %relation;
my $nNodes = 0;
for(my $i = 0; $i < scalar @lines; $i++){
    if( $lines[$i] =~ /node/ ){ $nNodes++; }
    my @line = split('\s', $lines[$i]);
    if(scalar @line > 1 && $line[1] eq "node"){
	my @ids = split('\s', $lines[$i+1]);
	my @labels = split('\s', $lines[$i+2]);
	$i += 2;
	$relation{$ids[3]} = $labels[3];
    }
}


my $seed = int(rand(999999999));
my $outFile = $file;
$outFile =~ s/\.gml//;
system("./src/RN/rnMRA -n:$nNodes -nsm:$nNodes -rseed:$seed -inf:$file -outf:$outFile > /dev/null");

system("rm $outFile.csv");
$outFile .= "Best.txt";
open(O, $outFile) or die "Something went wrong";
@lines = <O>;
close O;

my %part;
for(my $i = 0; $i < scalar @lines; $i++){
    my @tmp = split '\s', $lines[$i];
    foreach(@tmp){
	$part{$relation{$_}} = $i;
    }
}

system("rm $file");
$file =~ s/\.gml/\_RN.part/;
open F, ">$file";
print F "RN\n";
foreach(sort keys %part){
    print F "$_\t".$part{$_}."\n";
}
close F;

system("rm $outFile");
