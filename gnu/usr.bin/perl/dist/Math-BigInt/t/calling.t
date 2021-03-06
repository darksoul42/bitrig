#!/usr/bin/perl -w

# test calling conventions, and :constant overloading

use strict;
use Test::More tests => 160;

BEGIN { unshift @INC, 't'; }

package Math::BigInt::Test;

use Math::BigInt;
use vars qw/@ISA/;
@ISA = qw/Math::BigInt/;		# child of MBI
use overload;

package Math::BigFloat::Test;

use Math::BigFloat;
use vars qw/@ISA/;
@ISA = qw/Math::BigFloat/;		# child of MBI
use overload;

package main;

use Math::BigInt try => 'Calc';
use Math::BigFloat;

my ($x,$y,$z,$u);
my $version = '1.76';	# adjust manually to match latest release

###############################################################################
# check whether op's accept normal strings, even when inherited by subclasses

# do one positive and one negative test to avoid false positives by "accident"

my ($func,@args,$ans,$rc,$class,$try);
while (<DATA>)
  {
  $_ =~ s/[\n\r]//g;	# remove newlines
  next if /^#/; # skip comments
  if (s/^&//)
    {
    $func = $_;
    }
  else
    {
    @args = split(/:/,$_,99);
    $ans = pop @args;
    foreach $class (qw/
      Math::BigInt Math::BigFloat Math::BigInt::Test Math::BigFloat::Test/)
      {
      $try = "'$args[0]'"; 			# quote it
      $try = $args[0] if $args[0] =~ /'/;	# already quoted
      $try = '' if $args[0] eq '';		# undef, no argument
      $try = "$class\->$func($try);";
      $rc = eval $try;
      print "# Tried: '$try'\n" if !is ($rc, $ans);
      }
    } 

  }

$class = 'Math::BigInt';

# XXX TODO this test does not work/fail.
# test whether use Math::BigInt qw/version/ works
#$try = "use $class ($version.'1');";
#$try .= ' $x = $class->new(123); $x = "$x";';
#eval $try;
#is ( $x, undef );               # should result in error!

# test whether fallback to calc works
$try = "use $class ($version,'try','foo, bar , ');";
$try .= "$class\->config()->{lib};";
$ans = eval $try;
like ( $ans, qr/^Math::BigInt::(Fast)?Calc\z/);

# test whether constant works or not, also test for qw($version)
# bgcd() is present in subclass, too
$try = "use Math::BigInt ($version,'bgcd',':constant');";
$try .= ' $x = 2**150; bgcd($x); $x = "$x";';
$ans = eval $try;
is ( $ans, "1427247692705959881058285969449495136382746624");

# test whether Math::BigInt::Scalar via use works (w/ dff. spellings of calc)
$try = "use $class ($version,'lib','Scalar');";
$try .= ' $x = 2**10; $x = "$x";';
$ans = eval $try; is ( $ans, "1024");
$try = "use $class ($version,'lib','$class\::Scalar');";
$try .= ' $x = 2**10; $x = "$x";';
$ans = eval $try; is ( $ans, "1024");

# all done

__END__
&is_zero
1:0
0:1
&is_one
1:1
0:0
&is_positive
1:1
-1:0
&is_negative
1:0
-1:1
&is_nan
abc:1
1:0
&is_inf
inf:1
0:0
&bstr
5:5
10:10
-10:-10
abc:NaN
'+inf':inf
'-inf':-inf
&bsstr
1:1e+0
0:0e+1
2:2e+0
200:2e+2
-5:-5e+0
-100:-1e+2
abc:NaN
'+inf':inf
&babs
-1:1
1:1
&bnot
-2:1
1:-2
&bzero
:0
&bnan
:NaN
abc:NaN
&bone
:1
'+':1
'-':-1
&binf
:inf
'+':inf
'-':-inf
