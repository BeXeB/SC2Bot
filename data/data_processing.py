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
        self.labels = data['result']
        self.features = data.drop('result', axis=1)
        self.transform = transform
        self.target_transform = target_transform

    def __len__(self):
        return len(self.data)

    def __getitem__(self, idx):
        label = self.labels[idx]
        features = self.features[:idx]


class NeuralNetwork(nn.Module):
    def __init__(self):
        super(NeuralNetwork, self).__init__()
        self.input = nn.Linear(69, 128)
        self.hidden = nn.Linear(128, 64)
        self.output = nn.Linear(64, 3)

    def forward(self, x):
        x = x.view(-1, 69)
        x = F.relu(self.input(x))
        x = F.relu(self.hidden(x))
        x = F.log_softmax(self.output(x), dim=1)
        return x

# reading csv file
df = pd.read_csv("micro_arena.csv")
# removes columns that does not have any other values than 0
df = df.loc[:,(df != 0).any()]

print(df)

'''
Maybe do some kind of normalization
https://www.baeldung.com/cs/normalizing-inputs-artificial-neural-network
  - Min-Max Scaling
  - Z-Score Normalization
  - Proportional Normalization
  - Batch normalization
'''

train_dataset, test_dataset = torch.utils.data.random_split(df, [0.8, 0.2])

print("Number of rows in test data: " + str(len(test_dataset)))
print("Number of rows in train data: " + str(len(train_dataset)))

model = NeuralNetwork()

loss_function = nn.NLLLoss()
optimizer = optim.Adam(model.parameters(), lr=0.001)

epochs = 5
# for epoch in range(epochs):
#     for images, labels in train_loader:
#         optimizer.zero_grad()
#
#         output = model(images)
#         loss = loss_function(output, labels)
#
#         loss.backward()
#         optimizer.step()
#
#     print(f'Epoch [{epoch + 1}/{epochs}], Loss: {loss.item():.4f}')