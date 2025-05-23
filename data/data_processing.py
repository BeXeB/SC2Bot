import pandas as pd
import torch
import torch.nn as nn
import torch.nn.functional as F
import torch.optim as optim
import numpy as np
from torch.utils.data import Dataset


class MicroArenaDataset(Dataset):
    def __init__(self, data:pd.DataFrame, transform=None, target_transform=None):
        self.data:pd.DataFrame = data
        self.labels:torch.Tensor = torch.tensor(data['result'].to_numpy(), dtype=torch.long)

        features_df = data.drop('result', axis=1)
        self.features:torch.Tensor = torch.tensor(features_df.to_numpy(), dtype=torch.float32)

        self.transform = transform
        self.target_transform = target_transform

    def __len__(self):
        return len(self.data)

    def __getitem__(self, idx):
        label = self.labels[idx]
        # The whole row of features for index idx
        features = self.features[idx]
        if self.transform:
            features = self.transform(features)
        if self.target_transform:
            label = self.target_transform(label)
        return features, label


class ArenaNetwork(nn.Module):
    def __init__(self, input_size: int):
        super(ArenaNetwork, self).__init__()
        self.input_size = input_size
        self.input = nn.Linear(input_size, 128)
        self.hidden = nn.Linear(128, 64)
        self.output = nn.Linear(64, 3)

    def forward(self, x):
        x = x.view(-1, self.input_size)
        x = F.relu(self.input(x))
        x = F.relu(self.hidden(x))
        x = F.log_softmax(self.output(x), dim=1)
        return x

def createNN(race: str):
    filepath = race + "_micro_arena.csv"

    # reading csv file
    df = pd.read_csv(filepath)

    # Shuffle the data
    df = df.sample(frac=1).reset_index(drop=True)

    # Remap Creep feature: False -> 0, True -> 1
    df['on_creep'] = df['on_creep'].map({False: 0, True: 1})
    # Remap result labels: -1 -> 0 (loss), 0 -> 1 (tie), 1 -> 2 (win)
    df['result'] = df['result'].map({-1: 0, 0: 1, 1: 2})

    print(df)

    '''
    Maybe do some kind of normalization
    https://www.baeldung.com/cs/normalizing-inputs-artificial-neural-network
      - Min-Max Scaling
      - Z-Score Normalization
      - Proportional Normalization
      - Batch normalization
    '''

    dataset = MicroArenaDataset(df)
    train_set, test_set = torch.utils.data.random_split(dataset, [0.8, 0.2])

    train_loader = torch.utils.data.DataLoader(train_set, batch_size=32, shuffle=True)
    test_loader = torch.utils.data.DataLoader(test_set, batch_size=32, shuffle=False)

    print("Number of rows in test data: " + str(len(test_set)))
    print("Number of rows in train data: " + str(len(train_set)))

    print(f"Shape of the images in the training dataset: {train_loader.dataset[0][0].shape}")

    input_size = len(df.columns) - 1
    model = ArenaNetwork(input_size=input_size)

    loss_function = nn.NLLLoss()
    optimizer = optim.Adam(model.parameters(), lr=0.001)

    epochs = 30
    for epoch in range(epochs):
        for features, labels in train_loader:
            optimizer.zero_grad()

            output = model(features)
            loss = loss_function(output, labels)

            loss.backward()
            optimizer.step()

        print(f'Epoch [{epoch + 1}/{epochs}], Loss: {loss.item():.4f}')

    correct = 0
    total = 0
    with torch.no_grad():
        for features, labels in test_loader:
            output = model(features)
            _, predicted = torch.max(output, 1)
            total += labels.size(0)
            correct += (predicted == labels).sum().item()


    print(f'Accuracy of the neural network on the {total} test battles: {100 * correct / total}%')

    example_input = test_loader.dataset[0][0].unsqueeze(0)
    traced_model = torch.jit.trace(model, example_input)


    traced_model.save(race + "_arena_model.pt")
    torch.save(model.state_dict(), (race + '_arena_model.pth'))

if __name__ == "__main__":
    createNN("terran")
    createNN("protoss")
    createNN("zerg")