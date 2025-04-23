import pandas as pd

# reading csv file
df = pd.read_csv("micro_arena.csv")
# removes columns that does not have any other values than 0
df = df.loc[:,(df != 0).any()]

