#!/usr/bin/perl

use strict;
use warnings;

die "No input file specified!" unless @ARGV;
my $file = $ARGV[0];
my %n;
open F,$file or die "File $file not found!";
while(<F>){
    my ($a, $b) = split '\s', $_;
    $n{$a} = $n{$b} = 1;
}
close F;

my $nNodes = scalar keys %n;
my $it = 10000;
my $cmd = "./src/Jerarca/jerarca $file scluster $it";

system($cmd);


$file =~ s/\.\w+$//;
open F, "partition_scluster.txt" or die "Something went wrong!";
my %comm;
while(<F>){
    if(!($_ =~ /scluster/)){
	my ($a,$b) = split '\s', $_;
	$comm{$a} = $b;
    }
}
close F;

open F, ">$file\_SCluster.part";
print F "SCluster\n";
foreach(sort keys %comm){
    print F "$_\t".$comm{$_}."\n";
}
close F;

system("rm partition_scluster.txt");
