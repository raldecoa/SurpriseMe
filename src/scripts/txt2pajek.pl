#!/usr/bin/perl

# Copyright 2012 Rodrigo Aldecoa and Ignacio Marín
#  Contact: raldecoa@ibv.csic.es

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


if(scalar @ARGV != 1){
    print "Usage: ./txt2pajek.pl network_file\n";
    die;
}

my $pairsFile = $ARGV[0];

#Read tab-delimited file
open ( F, $pairsFile ) or die "$pairsFile not found!";
my @edges = <F>;
close F;

my ( %nodes, %edges );
foreach( @edges ){
    my @line = split( '\s', $_ );
    $nodes{$line[0]} = $nodes{$line[1]} = 1;
}
my %sorted;
my $index = 1;
foreach( sort keys %nodes ){
    $sorted{$_} = $index;
    $index++;
}
my $E = 0; 
foreach( @edges ){
    my @line = split( '\s', $_ );
    $edges{$sorted{$line[0]}}{$sorted{$line[1]}} = 1;
    $E++;
}


#Write graph to Pajek file
my $outFile = $pairsFile;
$outFile =~ s/\.\w+$/.pajek/;

open( OUT, ">$outFile" );
print OUT "*Vertices ".scalar( keys %nodes )."\n"; 
foreach( sort keys %sorted ){
    print OUT $sorted{$_}." \"$_\"\n"; }
print OUT "*Edges $E\n";
foreach my $k1 ( sort {$a <=> $b} keys %edges ){
    foreach my $k2 ( sort {$a <=> $b} keys %{ $edges{$k1} } ){
	print OUT "$k1 $k2\n";
    }
}
close OUT;
