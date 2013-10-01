#!/usr/bin/perl

# Copyright 2012 Rodrigo Aldecoa and Ignacio Marín
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

if(scalar @ARGV != 1){
    print "Usage: ./txt2gml.pl network_file\n";
    die;
}

my $pairsFile = $ARGV[0];

#Read tab-delimited file
my ( %nodes, %edges );
my $index = 0;
open ( F, $pairsFile ) or die "$pairsFile not found!";
while(<F>){
    my ($A, $B) = split( '\s', $_ );
    if(!exists($nodes{$A})){
        $nodes{$A} = $index;
        $index++;
    }
    if(!exists($nodes{$B})){
        $nodes{$B} = $index;
        $index++;
    }
    $edges{$nodes{$A}}{$nodes{$B}} = 1;
}
close F;


#Write network to gml file
my $outFile = $pairsFile;
$outFile =~ s/\.\w+$/.gml/;

open OUT, ">$outFile";
print OUT "graph[\n";
foreach( sort keys %nodes ){
    print OUT "\tnode [\n\t\tid ".$nodes{$_}."\n\t\tlabel $_\n]\n";
}

foreach my $k1 ( sort {$a <=> $b} keys %edges ){
    foreach my $k2 ( sort {$a <=> $b} keys %{ $edges{$k1} } ){
        print OUT "\tedge [\n\t\tsource $k1\n\t\ttarget $k2\n";
        print OUT "\t\tvalue 1\n\t]\n";
    }
}
print OUT "]\n";
close OUT;
