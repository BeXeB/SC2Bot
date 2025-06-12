import pandas as pd


def proccess_opponents(df : pd.DataFrame):
    opponents = df["Opponent"].unique().tolist()
    data = {'Opponent': [], 'Win': [], 'Loss': [], 'Tie': []}
    for opponent in opponents:
        data['Opponent'].append(opponent)
        opponent_df = df[df["Opponent"] == opponent]
        win_percentage = (opponent_df["Result"] == "win").mean()*100
        data['Win'].append(win_percentage)
        loss_percentage = (opponent_df["Result"] == "loss").mean()*100
        data['Loss'].append(loss_percentage)
        tie_percentage = (opponent_df["Result"] == "tie").mean()*100
        data['Tie'].append(tie_percentage)
    return pd.DataFrame(data)



def proccess_total(df, end_probability_function, army_value_function):
    win_percentage = (df["Result"] == "win").mean() * 100
    loss_percentage = (df["Result"] == "loss").mean() * 100
    tie_percentage = (df["Result"] == "tie").mean() * 100
    data = {'Army_value_function':[army_value_function],
            'End_probability_function': [end_probability_function],
            'Win': [win_percentage],
            'Loss': [loss_percentage],
            'Tie': [tie_percentage]}
            #, 'Elo': []}

    return pd.DataFrame(data)


def process_elo(df):
    pass
    # elo = process_elo(df)


def process_configuration(end_probability_function: int, army_value_function: str) -> (pd.DataFrame, pd.DataFrame):
    path = "combinedResults.csv"
    df = pd.read_csv(path)
    df = df.drop(columns=['Match'])
    df = df[df["Result"] != "error"]
    df = df[df["EndProbabilityFunction"] == end_probability_function]
    df = df[df["ArmyValueFunction"] == army_value_function]

    opponent_df = proccess_opponents(df)
    opponent_df.to_csv("match_results/" + str(end_probability_function) + "_" + str(army_value_function) + ".csv", index=False)
    total_df = proccess_total(df, end_probability_function ,army_value_function)

    return opponent_df, total_df

def process_data():
    total_df = pd.DataFrame({'Army_value_function':[],'End_probability_function': [],'Win': [],'Loss': [],'Tie': []})
    total_df['End_probability_function'] = total_df['End_probability_function'].astype(int)

    epf = 0
    avf = "marinePower"
    (opponent_df, configuration_df) = process_configuration(epf, avf)
    print_opponent_df(avf, epf, opponent_df)
    total_df = pd.concat([total_df, configuration_df])

    epf = 0
    avf = "minPower"
    (opponent_df, configuration_df) = process_configuration(epf, avf)
    print_opponent_df(avf, epf, opponent_df)
    total_df = pd.concat([total_df, configuration_df])

    epf = 0
    avf = "averagePower"
    (opponent_df, configuration_df) = process_configuration(epf, avf)
    print_opponent_df(avf, epf, opponent_df)
    total_df = pd.concat([total_df, configuration_df])

    epf = 0
    avf = "scaledPower"
    (opponent_df, configuration_df) = process_configuration(epf, avf)
    print_opponent_df(avf, epf, opponent_df)
    total_df = pd.concat([total_df, configuration_df])


    epf = 1
    avf = "marinePower"
    (opponent_df, configuration_df) = process_configuration(epf, avf)
    print_opponent_df(avf, epf, opponent_df)
    total_df = pd.concat([total_df, configuration_df])

    epf = 1
    avf = "minPower"
    (opponent_df, configuration_df) = process_configuration(epf, avf)
    print_opponent_df(avf, epf, opponent_df)
    total_df = pd.concat([total_df, configuration_df])

    epf = 1
    avf = "averagePower"
    (opponent_df, configuration_df) = process_configuration(epf, avf)
    print_opponent_df(avf, epf, opponent_df)
    total_df = pd.concat([total_df, configuration_df])

    epf = 1
    avf = "scaledPower"
    (opponent_df, configuration_df) = process_configuration(epf, avf)
    print_opponent_df(avf, epf, opponent_df)
    total_df = pd.concat([total_df, configuration_df])


    epf = 2
    avf = "marinePower"
    (opponent_df, configuration_df) = process_configuration(epf, avf)
    print_opponent_df(avf, epf, opponent_df)
    total_df = pd.concat([total_df, configuration_df])

    epf = 2
    avf = "minPower"
    (opponent_df, configuration_df) = process_configuration(epf, avf)
    print_opponent_df(avf, epf, opponent_df)
    total_df = pd.concat([total_df, configuration_df])

    epf = 2
    avf = "averagePower"
    (opponent_df, configuration_df) = process_configuration(epf, avf)
    print_opponent_df(avf, epf, opponent_df)
    total_df = pd.concat([total_df, configuration_df])

    epf = 2
    avf = "scaledPower"
    (opponent_df, configuration_df) = process_configuration(epf, avf)
    print_opponent_df(avf, epf, opponent_df)
    total_df = pd.concat([total_df, configuration_df])


    total_df[['Win', 'Loss', 'Tie']] = total_df[['Win', 'Loss', 'Tie']].round(2)
    total_df['End_probability_function'] = total_df['End_probability_function'].map({0: 'a', 1: 'b', 2: 'c'})
    print(total_df)
    return total_df
def print_opponent_df(avf, epf, opponent_df):
    print("End probability function: " + str(epf))
    print("Army value function: " + str(avf))
    print("------------------------------------------------")
    print(opponent_df)
    print("------------------------------------------------")


total_df = process_data()
total_df.to_csv("result_averages.csv", index=False)
