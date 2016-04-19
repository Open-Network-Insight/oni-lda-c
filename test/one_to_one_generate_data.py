__author__ = 'nlsegerl'

import argparse

default_topic_count = 3

if __name__ == "__main__":

    default_output_path = '/tmp/oni_ldac_test_one_to_one/one_to_one.dat'

    DOC_STRING = """Validates the output of the three topic test: Three docs, three words, three topics, three processes.
    Can LDA handle this trivial situation?
    """

    parser = argparse.ArgumentParser(description=DOC_STRING)
    parser.add_argument("-o", "--output", type=str,
                        help="Path to output file, relative to user directory. Defaults to " + default_output_path)


    parser.add_argument("-t", "--topic_count", type=int,
                        help="Number of topics used to generate corpus; will use one unique word per topic. Default:  " + str(default_topic_count))


    args = parser.parse_args()

    outpath = args.output if args.output else default_output_path
    num_topics  = args.topic_count if args.topic_count else default_topic_count

    f = open(outpath,'w')

    for i in range(0, num_topics):
        indicator_vector = ' '.join(map(lambda x: (str(x) + (":0" if x != i else ":1000")), range(0,num_topics)))
        f.write(str(num_topics) + ' ' + indicator_vector + '\n')

    f.close()
