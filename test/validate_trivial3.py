import sys
import argparse
import math
epsilon = 0.01

num_words = 4


def near_zero_count(coll):
    return len([x  for x in coll if  math.fabs(x) < epsilon])

def near_one_count(coll):
    return len([x for x in coll if math.fabs(1-x) < epsilon])

def topic_mix_bad(wmix):
    return (near_one_count(wmix) != 1 or near_zero_count(wmix) != (num_words-1))


def test_beta(test_dir):

    beta_path = test_dir + '/' + 'final.beta'
    f = open(beta_path, 'r')

    lines = [ l.strip() for l in f.readlines()]

    test_passed = True

    if (len(lines) != 3):
        print "TEST FAILED: " + beta_path + " should contain three lines, one per topic."
        test_passed = False
    else:
        words_per_topic = [[math.exp(float(entry)) for entry in line.split(' ')] for line in lines]

        if ([word_mix for word_mix in words_per_topic if len(word_mix) != num_words] != []):
            test_passed = False
            print "TEST FAILED: all rows of " + beta_path + " should contain " + str(num_words) + " many entries."
        else:
            if ([word_mix for word_mix in words_per_topic if topic_mix_bad(word_mix)] != []):
                test_passed = False
                print "TEST FAILED: all rows of " + beta_path + " should coutain exactly one entry within " \
                      + str(epsilon) + " of 1 and exactly " + str(num_words -1) + \
                      " manys entries within " + str(epsilon) + " of 0.0"
            else:
                topic1_word = [ w for w in range(0,num_words) if math.fabs(1-words_per_topic[0][w]) < epsilon][0]
                topic2_word = [ w for w in range(0,num_words) if math.fabs(1-words_per_topic[1][w]) < epsilon][0]
                topic3_word = [ w for w in range(0,num_words) if math.fabs(1-words_per_topic[2][w]) < epsilon][0]
                if (topic1_word == topic2_word or topic1_word == topic3_word or topic2_word == topic3_word):
                    test_passed = False
                    print "TEST FAILED:  Different topics should have their topic mixes centered on different words."
    f.close()
    return test_passed




if __name__ == "__main__":

    default_test_dir = '/tmp/oni_ldac_test_trivial3'

    DOC_STRING = """Validates the output of the three topic test: Three docs, three words, three topics, three processes.
    Can LDA handle this trivial situation?
    """

    parser = argparse.ArgumentParser(description=DOC_STRING)
    parser.add_argument("-i", "--input", type=str,
                        help="Path to input directory, relative to user directory. Defaults to " + default_test_dir)


    args = parser.parse_args()

    test_dir = args.input if args.input else default_test_dir

    test_passed  = test_beta(test_dir)

    if test_passed:
        print "three topic test: PASSED"
        sys.exit(0)
    else:
        print "three topic test: FAILED"
        sys.exit(-1)

