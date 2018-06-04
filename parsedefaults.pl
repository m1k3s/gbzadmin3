#!/usr/bin/perl
# this script operates on bzflag/src/common/global.cxx
$filename = @ARGV[0];

open (GLOBALCXX, $filename) || die "can't open file: $!";

while (<GLOBALCXX>) {
	chomp; # remove the newline
	@values = split("[\"|\",]");
	
	foreach my $val (@values) {
    print "$val\n";
  }

#	write;
}
close(GLOBALCXX);

#format STDOUT = 
#@<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<|  @>>>>>>>>>>>>>>>|  @>>>>>>>|     @>>>>|    @>>>>>>>>>|   @<<<<<<<<<<
#$callsign, $ip, $team, $score, $dur, $date
#.

#format STDOUT_TOP =
#                                       Page @<<<
#$%

#          Callsign                      IP             Team        Score       Duration      Date
#=================================  =================  =========   ========    ==========   ============
#.
