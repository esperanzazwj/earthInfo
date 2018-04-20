#!/usr/bin/env perl
use utf8;

$base_dir__source = "source";
$build__output_bin = "ss-engine";
$build__main_impl = "main.cpp";
$compiler__flags__standard = "c++14";
$compiler__flags__optimization_level = "3";
@compiler__flags__extra = qw[
    -Wall -Wextra -Werror
    -Wno-missing-field-initializers
    -Wno-unused-parameter
    -Wno-unused-private-field

    -DNDEBUG

    -I include
    -I source/override/assert
];

my $target_linux = 0;
my $enable_debug = 0;
$makefile__show_commands = 0;

for (@ARGV) {
    $_ eq "linux" and do {
        $target_linux = 1;
        next;
    };

    $_ eq "debug" and do {
        $enable_debug = 1;
        next;
    };

    $_ eq "verbose" and do {
        $makefile__show_commands = 1;
        next;
    };

    $_ eq "small" and do {
        $compiler__flags__optimization_level = "s";
        next;
    };
}

if ($target_linux) {
    $compiler__bin = "g++";
    $build__output_bin_dir = "bin";
    @library__pkg_config = qw[
        gl
        glew
        glfw3
    ];
    @library__link = qw[
        freeimage
        assimp
    ];
    push @compiler__flags__extra, qw[
        -Wno-terminate
        -g
    ];
    @makefile__commands__test = (
        {
            name => "ss Engine",
            commands => [
                "(cd \$(BIN_DIR); ./$build__output_bin)",
            ],
        },
    );
}
else {
    $compiler__bin = "em++";
    $build__output_bin_suffix = ".html";
    @build__output_bin_extra_suffixes = qw[.js .data .wasm];
    $build__output_bin_dir = "build/web";
    $compiler__flags__architecture = undef;
    push @compiler__flags__extra, "-Wno-inconsistent-missing-override";    # FIXME: fix this properly
    @loader__flags__extra = qw[
        -s BINARYEN=1
        -s DISABLE_EXCEPTION_CATCHING=0
        -s USE_GLFW=3
        -s USE_WEBGL2=1
        -s FULL_ES3=1

        -s ALLOW_MEMORY_GROWTH=1
        --no-heap-copy

        --preload-file bin/runtime@runtime
        --shell-file web/shell.html

        lib/free-image/freeimage.bc
        lib/zlib/z.bc
        lib/assimp/assimp.bc
    ];
    push @loader__flags__extra, "-O$compiler__flags__optimization_level";
    if ($enable_debug) {
        push @compiler__flags__extra, "-g";
        push @loader__flags__extra, "-g";
    }
    @makefile__commands__test = (
        {
            name => "Server",
            commands => [
                "~/.cargo/bin/simple-http-server --ip 0.0.0.0 -p 4000 --cors -iu $build__output_bin_dir",
            ],
        },
    );
}

$loader__bin = $compiler__bin;

