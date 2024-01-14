#! /usr/bin/env bats
# To use this file make sure type: make lexer_test at ../

# Number cases ----------------------------------------------------------------
# Note: This lexer does not know -43 is a number since - is a token by itself
#       and later on in the program it will make sense to the parser.
@test "testing lexer (1) [number]" {
    run="$(./lexer_test 32)"
    [ "$run" = "32.0" ]
}

@test "testing lexer (2) [number]" {
    run="$(./lexer_test "432.1e2")"
    [ "$run" = "43210.0" ]
}

@test "testing lexer (3) [number]" {
    run="$(./lexer_test "3.1415e0")"
    [ "$run" = "3.1" ]
}

@test "testing lexer (4) [number]" {
    run="$(./lexer_test "1234567890")"
    [ "$run" = "1234567890.0" ]
}

@test "testing lexer (5) [number]" {
    run="$(./lexer_test "0.00000001e15")"
    [ "$run" = "10000000.0" ]
}

# String cases ----------------------------------------------------------------
# Note: The lexer only accpets 32 bytes per string, if the sting is longer it will
#       skip the bytes beyond such limit.
@test "testing lexer (1) [string]" {
    run="$(./lexer_test "\"Bonjour\"")"
    [ "$run" = "Bonjour" ]
}

@test "testing lexer (2) [string]" {
    run="$(./lexer_test "\"1234567890\"")"
    [ "$run" = "1234567890" ]
}

@test "testing lexer (3) [string]" {
    run="$(./lexer_test "\"Cette fille est trop belle\"")"
    [ "$run" = "Cette fille est trop belle" ]
}

@test "testing lexer (4) [string]" {
    run="$(./lexer_test "\"12345678901234567890123456789012\"")"
    [ "$run" = "12345678901234567890123456789012" ]
}

@test "testing lexer (5) [string]" {
    run="$(./lexer_test "\"C'est ca!, That's it!, Eso es!\"")"
    [ "$run" = "C'est ca!, That's it!, Eso es!" ]
}

# References address cases ----------------------------------------------------
# Note: Since the reference token is not a literal value but a pointer to a cell
#       what the tester does is to check the row and column positions are what they are supposed to be.
@test "testing lexer (1) [reference]" {
    run="$(./lexer_test "&A0")"
    [ "$run" = "(0, 0)" ]
}

@test "testing lexer (2) [reference]" {
    run="$(./lexer_test "&Z15")"
    [ "$run" = "(25, 15)" ]
}

@test "testing lexer (3) [reference]" {
    run="$(./lexer_test "&AZ100")"
    [ "$run" = "(51, 100)" ]
}

@test "testing lexer (4) [reference]" {
    run="$(./lexer_test "&BDS164")"
    [ "$run" = "no-well-defined-ref" ]
}

@test "testing lexer (5) [reference]" {
    run="$(./lexer_test "&BZ111")"
    [ "$run" = "(77, 111)" ]
}

# Literal combined cases ------------------------------------------------------
@test "testing lexer (1) [combined]" {
    run="$(./lexer_test "1 1 \"Hi!\" \"Hola!\" \"Bonsoir!\" &B1")"
    expected=$(echo -e "1.0\n1.0\nHi!\nHola!\nBonsoir!\n(1, 1)")
    [[ "$run" == "$expected" ]]
}

@test "testing lexer (2) [combined]" {
    run="$(./lexer_test "1 2 3 4 5 6 7 8 9 0")"
    expected=$(echo -e "1.0\n2.0\n3.0\n4.0\n5.0\n6.0\n7.0\n8.0\n9.0\n0.0")
    [[ "$run" == "$expected" ]]
}

@test "testing lexer (3) [combined]" {
    run="$(./lexer_test "&A0 &A1 &C1 &D0 &")"
    expected=$(echo -e "(0, 0)\n(0, 1)\n(2, 1)\n(3, 0)\nno-well-defined-ref")
    [[ "$run" == "$expected" ]]
}

@test "testing lexer (4) [combined]" {
    run="$(./lexer_test "\"Debut decembre, a Buenos Aires\" \"Un soir d'ete, sur l'autre hemisphere\"")"
    expected=$(echo -e "Debut decembre, a Buenos Aires\nbig string!")
    [[ "$run" == "$expected" ]]
}

@test "testing lexer (5) [combined]" {
    run="$(./lexer_test "\"Lexing is working\" 10 \"outta\" 10")"
    expected=$(echo -e "Lexing is working\n10.0\noutta\n10.0")
    [[ "$run" == "$expected" ]]
}

# Formulas cases --------------------------------------------------------------
@test "testing lexer (1) [formulas]" {
    run="$(./lexer_test "@sin(@pi / 2)")"
    expected=$(echo -e "@sin\n(\n@pi\n/\n2.0\n)")
    [[ "$run" == "$expected" ]]
}

@test "testing lexer (2) [formulas]" {
    run="$(./lexer_test "@sin(@pi) / @cos(@pi)")"
    expected=$(echo -e "@sin\n(\n@pi\n)\n/\n@cos\n(\n@pi\n)")
    [[ "$run" == "$expected" ]]
}

@test "testing lexer (3) [formulas]" {
    run="$(./lexer_test "@sqrt(@acos(0))")"
    expected=$(echo -e "@sqrt\n(\n@acos\n(\n0.0\n)\n)")
    [[ "$run" == "$expected" ]]
}

@test "testing lexer (4) [formulas]" {
    run="$(./lexer_test "@sin(2 * @pi) * @sin(2 * @pi) + @cos(2 * @pi) * @cos(2 * @pi)")"
    expected=$(echo -e "@sin\n(\n2.0\n*\n@pi\n)\n*\n@sin\n(\n2.0\n*\n@pi\n)\n+\n@cos\n(\n2.0\n*\n@pi\n)\n*\n@cos\n(\n2.0\n*\n@pi\n)")
    [[ "$run" == "$expected" ]]
}

@test "testing lexer (5) [formulas]" {
    run="$(./lexer_test "@sqrt @sin @asin @cos @acos @atan @pi @e")"
    expected=$(echo -e "@sqrt\n@sin\n@asin\n@cos\n@acos\n@atan\n@pi\n@e")
    [[ "$run" == "$expected" ]]
}

# Unknown token kind cases ----------------------------------------------------
@test "testing lexer (1) [unknown-cases]" {
    run="$(./lexer_test "unknasderujkpoaqwertjkld@some_fx")"
    expected="$(yes "unkn" | head -n 32)"
    [[ "$run" == "$expected" ]]
}


