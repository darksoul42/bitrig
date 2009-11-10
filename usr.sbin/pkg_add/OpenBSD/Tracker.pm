# ex:ts=8 sw=4:
# $OpenBSD: Tracker.pm,v 1.7 2009/11/10 10:28:12 espie Exp $
#
# Copyright (c) 2009 Marc Espie <espie@openbsd.org>
#
# Permission to use, copy, modify, and distribute this software for any
# purpose with or without fee is hereby granted, provided that the above
# copyright notice and this permission notice appear in all copies.
#
# THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
# WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
# MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
# ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
# WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
# ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
#

use strict;
use warnings;

# In order to deal with dependencies, we have to know what's actually installed,
# and what can actually be updated.
# Specifically, to solve a dependency:
# - look at packages to_install
# - look at installed packages
#   - if it's marked to_update, then we must process the update first
#   - if it's marked as installed, or as cant_update, or uptodate, then
#   we can use the installed packages.
#   - otherwise, in update mode, put a request to update the package (e.g.,
#   create a new UpdateSet.

# the Tracker object does maintain that information globally so that 
# Update/Dependencies can do its job.

package OpenBSD::Tracker;
sub new
{
	my $class = shift;
	return bless {}, $class;
}

sub add_set
{
	my ($self, $set) = @_;
	for my $n ($set->newer) {
		$self->{to_install}->{$n->pkgname} = $set;
	}
	for my $n ($set->older) {
		$self->{to_update}->{$n->pkgname} = $set;
	}
	for my $n ($set->hints) {
		$self->{to_update}->{$n} = $set;
	}
	return $self;
}

sub add_sets
{
	my ($self, @sets) = @_;
	for my $set (@sets) {
		$self->add_set($set);
	}
	return $self;
}

sub remove_set
{
	my ($self, $set) = @_;
	for my $n ($set->newer) {
		delete $self->{to_install}->{$n->pkgname};
	}
	for my $n ($set->older) {
		delete $self->{to_update}->{$n->pkgname};
	}
}

sub mark_uptodate
{
	my ($self, $set) = @_;
	for my $n ($set->older) {
		delete $self->{to_update}->{$n->pkgname};
		$self->{uptodate}->{$n->pkgname} = 1;
	}
}

sub mark_cant_update
{
	my ($self, $set) = @_;
	for my $n ($set->older) {
		delete $self->{to_update}->{$n->pkgname};
		$self->{cant_update}->{$n->pkgname} = 1;
	}
}

sub mark_installed
{
	my ($self, $set) = @_;
	for my $n ($set->newer) {
		$self->{installed}->{$n->pkgname} = 1;
	}
	$self->remove_set($set);
}

sub is_installed
{
	my ($self, $pkg) = @_;
	return $self->{installed}->{$pkg};
}

sub installed
{
	my $self = shift;
	return keys %{$self->{installed}};
}

sub cant_update
{
	my $self = shift;
	return keys %{$self->{cant_update}};
}

1;
