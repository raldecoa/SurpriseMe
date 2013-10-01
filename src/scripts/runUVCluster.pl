#!/usr/bin/perl

use strict;
use warnings;

die "No input file specified!" unless @ARGV;
my $file = $ARGV[0];
my $inputFile = $file;
my %n;
open F,$file or die "File $file not found!";
while(<F>){
    my ($a, $b) = split '\s', $_;
    $n{$a} = $n{$b} = 1;
}
close F;

my $nNodes = scalar keys %n;
my $it = 1000;
if(($nNodes*10) < $it){ $it = $nNodes * 10; }
my $cmd = "./src/Jerarca/jerarca $file uvcluster $it";

system($cmd);

$file =~ s/\.\w+$//;
open F, "partition_uvcluster.txt" or die "Something went wrong!";
my %comm;
while(<F>){
    if(!($_ =~ /uvcluster/)){
	my ($a,$b) = split '\s', $_;
	$comm{$a} = $b;
    }
}
close F;

my $part = "$file\_UVCluster.part";
open F, ">$part";
print F "UVCluster\n";
foreach(sort keys %comm){
    print F "$_\t".$comm{$_}."\n";
}
close F;
system("rm partition_uvcluster.txt");
