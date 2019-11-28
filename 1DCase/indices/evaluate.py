from btreeTest import BTreeEvaluator

import sys, os
sys.path.append(os.getcwd() + "/..")
import config

for distribution in config.Distribution:
    e = BTreeEvaluator(4)
    e.evaluate_dense_distribution(distribution)
    del e