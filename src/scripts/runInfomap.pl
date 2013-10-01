#!/usr/bin/perl

use strict;
use warnings;

die "No input file specified!" unless @ARGV;

my $file = $ARGV[0];
my $inputFile = $file;
my %nodes;
open F, $file or die "File $file not found!";
while(<F>){
    my ($a, $b) = split '\s', $_;
    $nodes{$a} = $nodes{$b} = 1;
}
close F;


system("./src/scripts/txt2pajek.pl $file");
$file =~ s/\.\w+$//;

#my $nNodes = scalar(keys %nodes);
my $r = int(rand(132153454));
my $attempts = 10;
system("./src/Infomap/src/infomap $r $file.pajek $attempts > /dev/null");

system("rm $file.clu $file.pajek $file\_map.*");

open F, "$file.tree" or die "Something went wrong!";
my %comm;
while(<F>){
    if( !($_ =~ /modules/) ){
	$_ =~ s/\"//g;
	my ($cc, $tmp, $n) = split '\s', $_;
	my ($c, $tmp2) = split ':', $cc;
	$comm{$n} = $c;
    }
}
close F;
system("rm $file.tree");

open O, ">$file\_Infomap.part";
print O "Infomap\n";
foreach(sort keys %comm){
    print O "$_\t".$comm{$_}."\n";
}
close O;
