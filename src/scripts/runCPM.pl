#!/usr/bin/perl

use strict;
use warnings;

my $file = $ARGV[0];
open F, $file or die "File $file not found!";
my @edges = <F>;
close F;

my $inputFile = $file;

my ( %node2index, %index2node );
my $index = 0;
open T, ">tmpCPM";
foreach(@edges){
    my ($a, $b) = split '\s', $_;
    if(!exists($node2index{$a})){
	$node2index{$a} = $index;
	$index2node{$index} = $a;
	$index++;
    }
    if(!exists($node2index{$b})){
	$node2index{$b} = $index;
	$index2node{$index} = $b;
	$index++;
    }
    print T $node2index{$a}."\t".$node2index{$b}."\n";
}
close T;


my %node;
for(my $i = 0; $i < scalar @edges; $i++){
    my @edge = split '\s', $edges[$i];
    $node{$edge[0]} = $node{$edge[1]} = 1;
}
my @nodes = sort keys %node;


open O, ">tmp2CPM";
print O ">\n";
for(my $i = 0; $i < scalar @nodes; $i++){
    print O "$i $i\n";
}
print O ">\n1 1\n>\n";
open I, "tmpCPM";
my @ed = <I>;
close I;
foreach my $e (@ed){
    my ($a, $b) = split '\s', $e;
    print O "$a $b 1 1\n";
}
close O;


#Compute gamma

my $bin = $file;
my $conf = $file;
$bin =~ s/\.\w+$/.bin/;
$conf =~ s/\.\w+$/.conf/;
system("./src/CPM/bin/slicer -C -i tmp2CPM -o $bin -c $conf -n tmpCPM 2> /dev/null");

my $gamma;
my $pastS = -1;
my $pastSS = -1;
my $notFound = 1;
my $sum = .5;
my $times = 100;
my $count = 0;
my $maxS = 0;
my $part;
while($notFound){
    for($gamma = 0.001; $gamma <= 1; $gamma += $sum){
	system("./src/CPM/bin/community $bin $conf -pp $gamma -o tmpCPM 2> /dev/null");
	
	open T, "tmpCPM" or die "Something went wrong!";
	my %comm;
	while(<T>){
	    my ($n, $c) = split '\s', $_;
	    $comm{$index2node{$n}} = $c;
	}
	close T;
	
	$part = $file;
	my $bestPart = $file;
	$part =~ s/\.\w+$/\_tmpCPM.part/;
	$bestPart =~ s/\.\w+$/\_CPM.part/;
	open T, ">$part";
	print T "CPM (gamma=$gamma)\n";
	foreach(sort keys %comm){
	    print T "$_\t".$comm{$_}."\n";
	}
	close T;
	
	system("./src/Surprise/computeSurprise $file $part > tmpCPM");
	my $S = &getS("tmpCPM");
	if($S > $maxS){
	    $count = 0;
	    $maxS = $S;
	    system("cp $part $bestPart");
	}
	else{
	    $count++;
	    if($count > $times){
		$notFound = 0;
		last;
	    }
	}
    }
    $sum /= 2;
}

system("rm tmp2CPM tmpCPM $part $bin $conf");


sub getS{
    my $f = shift @_;
    open F, $f or die "$f";
    my @lines = <F>;
    close F;

    if($lines[0] =~ /Surprise = (.*)$/){
	return $1;
    }
}

