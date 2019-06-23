#include <stdio.h>
#include "../hobig.c"

void test_gcd() {
    HoBigInt a = hobig_int_new_decimal("819745987109487895179874092387498132897219172095798127985798127985791279579", 0);
    HoBigInt b = hobig_int_new_decimal("90219309102930192093021322", 0);
    
    hobig_int_mul(&a, &b);

    HoBigInt gcd = hobig_int_gcd(&a, &b);

    hobig_int_print(gcd);
}

void miller_rabin_test() {
    u64 primes[] = {
        179424887,	179425357,	179425819,	179426321,
        179424893,	179425373,	179425823,	179426323,
        179424899,	179425399,	179425849,	179426333,
        179424907,	179425403,	179425859,	179426339,
        179424911,	179425423,	179425867,	179426341,
        179424929,	179425447,	179425879,	179426353,
        179424937,	179425453,	179425889,	179426363,
        179424941,	179425457,	179425907,	179426369,
        179424977,	179425517,	179425943,	179426407,
        179424989,	179425529,	179425993,	179426447,
        179425003,	179425537,	179426003,	179426453,
        179425019,	179425559,	179426029,	179426491,
        179425027,	179425579,	179426081,	179426549,
    };

    // All primes
    for(int i = 0; i < sizeof(primes) / sizeof(u64); ++i) {
        HoBigInt a = hobig_int_new(primes[i]);
        hobig_int_print(a);
        
        int prime = hobig_is_prime(&a, 2);

        if(prime) {
            printf(" is probably prime\n");
        } else {
            printf(" is composite\n");
        }
    }

    HoBigInt t = hobig_random_possible_prime(512);
    int prime = miller_rabin_probably_prime(&t, 20);
    hobig_int_print(t);
    printf(" is probably prime (%d)\n", hobig_int_bitcount(&t));
}

void test_random() {
    HoBigInt max = hobig_int_new_decimal("210391828052150182974921874891649786132789", 0);
    hobig_int_print(max);
    printf("\n\n");
    for(int i = 0; i < 10; ++i) {
        HoBigInt random = hobig_random(&max);
        hobig_int_print(random);
        printf("\n");
    }
}

void test_random_prime() {
    HoBigInt p = hobig_int_new_decimal("102985109289408084018290847480515927498732847912379471293498172359013956910", 0);
    HoBigInt two = hobig_int_new(2);
    //for(int i = 0; i < 1000; ++i) {
    while(1) {
        hobig_int_add(&p, &two);
        if(hobig_is_prime(&p, 20)) {
            hobig_int_print(p);
            printf(" is prime\n");
            break;
        }
    }

    // Product of small primes = 16294579238595022365
}

void test_prime_generate() {
    HoBigInt prime = hobig_random_possible_prime(128);
    hobig_int_print(prime);
}

void test_hobig_mod_div() {
    HoBigInt msg = hobig_int_new_decimal("1113867396759340784721330768221620586871924494590377436769198486702366330613553224719572658944953359272359304719274977693960922305278869063479600840525561228882038512200172199853522440845231575372106766028862622987481349447700589022122176964791228302065084300966324852121584646997049602778293396109510468250985098807088752611007064847882510437552487685041118637112056720794957244693536086195436302603314928195976916488458014385027516698723632096888914210014934072767479056228543895096225543487830813079147927708432560820371530236730916772916435839834194761105592904484141645380698017415152302901891788221841173615", 0);
    HoBigInt mod = hobig_int_new_decimal("21666702504857787886793638841817777498121126134302089344545705387038264104638108701946962254644439789069544779112431030802536249429364456978613012374820510911065695907417928646473918616160720523654431377042623126887404123197411400160326547299171020909417625243727890796840091859929623156916296551226209681481981576650632187297874777769601685623279503714849175951825203298008687580589518639830171068192340537060207061960546585056640181301195828416144172496217014188591013136765772758965747057573803612359717627543036296188888932613513340616975176414812691772576660438518958126167597118680937361950807754817355538540621", 0);
    HoBigInt private = hobig_int_new_decimal("17441931897268522717418539115562541850342163550261831173833747727380941111623927535549656447968801660004746074071343459772040335190722950719100131328421815381326639679624564437350977285371733423668469575812361123156776580124952784681302286989176567757740742978906511782042045964053390581085340699903754678664126178732696757016690202643029634595455638348279086126562627115128948213854799845614300885965528677383709202584083643208758490137979617692135848596898204551347341738254396136261472984008101960104180746076185534871464470826234874062205606728245358044874901187686973495209886935898874648833951499111553371140969", 0);
    HoBigInt pow = hobig_int_new_decimal("65537", 0);

    HoBigInt r = hobig_int_mod_div(&msg, &pow, &mod);

    hobig_int_print(r);
    printf("\n");
}

void test_mult() {
    HoBigInt d = hobig_int_new_decimal("1113867396759340784721330768221620586871924494590377436769198486702366330613553224719572658944953359272359304719274977693960922305278869063479600840525561228882038512200172199853522440845231575372106766028862622987481349447700589022122176964791228302065084300966324852121584646997049602778293396109510468250985098807088752611007064847882510437552487685041118637112056720794957244693536086195436302603314928195976916488458014385027516698723632096888914210014934072767479056228543895096225543487830813079147927708432560820371530236730916772916435839834194761105592904484141645380698017415152302901891788221841173615", 0);
    //hobig_int_print(d);
    hobig_int_mul(&d, &d);
    hobig_int_print(d);
    //printf("\n");
}
int main(int argc, char** argv) {
    double start = os_time_us();

    test_hobig_mod_div();

    //test_hobig_mod_div();
    //test_gcd();
    //miller_rabin_test();
    //test_random();
    //test_random_prime();
    //test_prime_generate();
    //test_addition_benchmark();
    //test_subtraction_benchmark();
    printf("\nelapsed: %.2fms\n", 0.001 * (os_time_us() - start));
    return 0;
}