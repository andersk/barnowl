#!/usr/bin/env perl
use strict;
use warnings;

use Test::More qw(no_plan);

use File::Basename;
BEGIN {require (dirname($0) . "/mock.pl");};

=head1 DESCRIPTION

Basic tests for tab-completion functionality.

=cut

sub test_tokenize {
    local $Test::Builder::Level = $Test::Builder::Level + 1;

    my $before_point = shift;
    my $after_point = shift;
    
    my $words = shift;
    my $word = shift;
    my $word_point = shift;

    my $word_start = shift;
    my $word_end   = shift;

    my $ctx = BarnOwl::Completion::Context->new($before_point,
                                                $after_point);

    is($ctx->line, $before_point . $after_point);
    is($ctx->point, length $before_point);
    is_deeply($ctx->words, $words);
    if (defined($word)) {
        is($ctx->word, $word, "Correct current word.");
        is($ctx->word_point, $word_point, "Correct point within word.");
        is($ctx->word_start, $word_start, "Correct start of word");
        is($ctx->word_end,   $word_end, "Correct end of word");
    }
}


new_ok('BarnOwl::Completion::Context' => ['Hello, W', 'orld']);

no warnings 'qw';
test_tokenize('Hello, W', 'orld',
              [qw(Hello, World)], 1, 1, 7, 12);

test_tokenize('Hello, World', '',
              [qw(Hello, World)], 1, 5, 7, 12);

test_tokenize('', '',
              [qw()], 0, 0, 0, 0);

test_tokenize('Hello', 'World',
              [qw(HelloWorld)], 0, 5, 0, 10);

test_tokenize('lorem ipsum dolor ', 'sit amet',
              [qw(lorem ipsum dolor sit amet)],
              3, 0, 18, 21);

test_tokenize(q{error "ls -l failed"}, q{},
              ['error', 'ls -l failed'],
              1, 12, 6, 20);

test_tokenize(q{"a long"' word'}, q{},
              ['a long word']);

test_tokenize(q{"'"}, q{}, [q{'}], 0, 1, 0, 3);

test_tokenize(q{"Hello, }, q{World"},
              [q{Hello, World}],
              0, 7, 0, 14);

test_tokenize(q{But 'Hello, }, q{World'},
              ['But', q{Hello, World}],
              1, 7, 4, 18);

test_tokenize(q{But "Hello, }, q{World"''''''""},
              ['But', q{Hello, World}],
              1, 7, 4, 26);

test_tokenize(q{}, q{''Hello},
              ['Hello'],
              0, 0, 0, 7);

test_tokenize(q{"Hello, }, q{World},
              [q{Hello, World}],
              0, 7, 0, 13);

test_tokenize(q{Hello    }, q{World},
              [qw{Hello World}],
              1, 0, 9, 14);

test_tokenize(q{Hello '' ""}, q{ World},
              ["Hello", '', '', 'World'],
              2, 0, 9, 11);

# It's not entirely clear what we should do here. Make a test for the
# current behavior, so we'll notice if it changes.
test_tokenize(q{Hello }, q{ World},
              [qw(Hello World)],
              1, -1, 7, 12);

1;
