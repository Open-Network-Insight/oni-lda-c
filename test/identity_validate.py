import sys
import argparse
import math
epsilon = 0.01

default_topic_count = 3


def near_zero_count(coll):
    return len([x  for x in coll if  math.fabs(x) < epsilon])

def near_one_count(coll):
    return len([x for x in coll if math.fabs(1-x) < epsilon])

def mix_not_a_near_unit(wmix,dimension):
    return (near_one_count(wmix) != 1 or near_zero_count(wmix) != (dimension-1))

def normalize(l):
    s = sum(l)
    return map(lambda x: float(x)/s, l)

def set_of_satisfiers(predicate, collection):
    return set(filter(predicate, collection))

def exists(predicate, collection):
    return len(filter(predicate, collection)) > 0


def indices_of_satisfiers(predicate, list):
    return set_of_satisfiers(lambda i: predicate(list[i]), range(0, len(list)))

def union(collection_of_sets):
    return reduce(lambda s,t: s.union(t), collection_of_sets)

def test_beta(test_dir, num_topics, num_words):

    beta_path = test_dir + '/' + 'final.beta'
    f = open(beta_path, 'r')

    lines = [ l.strip() for l in f.readlines()]

    test_passed = True

    if (len(lines) != num_topics):
        print "TEST FAILED: " + beta_path + " should contain "+ str(num_topics) + " lines, one per topic."
        test_passed = False
    else:
        words_per_topic = [[math.exp(float(entry)) for entry in line.split(' ')] for line in lines]

        if (exists(lambda word_mix: len(word_mix) != num_words, words_per_topic)):

            test_passed = False
            print "TEST FAILED: all rows of " + beta_path + " should contain " + str(num_words) + " many entries."
            bad = set_of_satisfiers(lambda word_mix: len(word_mix) != num_words, words_per_topic)
            example = bad.pop()
            print "Example bad row: " + str(example)

        else:
            if (exists(lambda word_mix: mix_not_a_near_unit(word_mix, num_words), words_per_topic)):
                test_passed = False
                print "TEST FAILED: all rows of " + beta_path + " should coutain exactly one entry within " \
                      + str(epsilon) + " of 1 and exactly " + str(num_words -1) + \
                      " manys entries within " + str(epsilon) + " of 0.0"
            else:
                words_in_topics = union([indices_of_satisfiers(lambda x: math.fabs(1-x) < epsilon, tmix) for tmix in words_per_topic])
                if (len(words_in_topics) != num_words):
                    test_passed = False
                    print "TEST FAILED:  Different topics should have their topic mixes centered on different words."
    f.close()
    return test_passed


def test_gamma(test_dir, num_topics, num_documents):

    gamma_path = test_dir + '/' + 'final.gamma'
    f = open(gamma_path, 'r')

    lines = [ l.strip() for l in f.readlines()]

    test_passed = True

    if (len(lines) != num_documents):
        print "TEST FAILED: " + gamma_path + " should contain " + str(num_documents) + " lines, one per document."
        test_passed = False
    else:
        unnormalized_topic_per_doc_vecs = [[float(entry) for entry in line.split(' ')] for line in lines]
        if (exists(lambda topic_mix: len(topic_mix) != num_topics, unnormalized_topic_per_doc_vecs)):
            test_passed = False
            print "TEST FAILED: all rows of " + gamma_path + " should contain " + str(num_topics) + " many entries."
            bad = set_of_satisfiers(lambda topic_mix: len(topic_mix) != num_topics, unnormalized_topic_per_doc_vecs)
            example = bad.pop()
            print "Example bad row: " + str(example)
        else:
            topic_per_doc_vecs = map(normalize, unnormalized_topic_per_doc_vecs)
            if (exists(lambda topic_mix: mix_not_a_near_unit(topic_mix, num_topics), topic_per_doc_vecs)):
                test_passed = False
                print "TEST FAILED: all rows of " + gamma_path + " should coutain exactly one entry within " \
                      + str(epsilon) + " of 1 and exactly " + str(num_topics -1) + \
                      " manys entries within " + str(epsilon) + " of 0.0"
            else:

                topics_in_docs = union([indices_of_satisfiers(lambda x: math.fabs(1-x) < epsilon, dmix) for dmix in topic_per_doc_vecs])
                if (len(topics_in_docs) != num_topics):
                    test_passed = False
                    print "TEST FAILED:  Different documents should have their topic mixes centered on different words."
    f.close()
    return test_passed

if __name__ == "__main__":

    default_test_dir = '/tmp/oni_ldac_test_identity'

    DOC_STRING = """Validates the output of the three topic test: Three docs, three words, three topics, three processes.
    Can LDA handle this trivial situation?
    """

    parser = argparse.ArgumentParser(description=DOC_STRING)
    parser.add_argument("-i", "--input", type=str,
                        help="Path to input directory, relative to user directory. Defaults to " + default_test_dir)


    parser.add_argument("-t", "--topic_count", type=int,
                        help= "Number of topics used to generate corpus; will use one unique word per topic. Default:  " \
                             + str(default_topic_count))


    args = parser.parse_args()

    test_dir = args.input if args.input else default_test_dir
    num_topics  = args.topic_count if args.topic_count else default_topic_count
    num_words   =  num_topics
    num_documents = num_topics

    test_passed  =  test_beta(test_dir, num_topics, num_words)
    test_passed  &= test_gamma(test_dir, num_topics, num_documents)

    if test_passed:
        print "three topic test: PASSED"
        sys.exit(0)
    else:
        print "three topic test: FAILED"
        sys.exit(-1)

