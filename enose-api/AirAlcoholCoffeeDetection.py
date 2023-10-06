import pickle
import sys
import pandas as pd

from collections import Counter

class AirAlcoholCoffeeDetection:

    # constructor
    def __init__(self, scalerFile, modelFile):
        # load the StandardScaler from disk
        self.loaded_sc = pickle.load(open(scalerFile, 'rb'))

        # load the model from disk
        self.loaded_model = pickle.load(open(modelFile, 'rb'))


    # create lists for X_test and y_test based on json data
    def formatData(self, jsonData):

        HUMIDITY = 4
        RESISTANCE = 5

        dataArray = jsonData['datapoints']

        # create dictionary
        data = {}
        hList = []
        rList = []

        for datapoint in dataArray:
            hList.append(datapoint[HUMIDITY])
            rList.append(datapoint[RESISTANCE])
       
        self.datapoint_count = len(rList)
        data['humidity'] = hList
        data['gas_resistance'] = rList

        # create dataframe using the dictionary
        df = pd.DataFrame(data)

        return df
    

    # predict the smell based on the json sample
    def PredictSmell(self, jsonData):
        X_test = self.formatData(jsonData)
        
        # transform features and make prediction
        X_testSc = self.loaded_sc.transform(X_test)
        y_pred = self.loaded_model.predict(X_testSc)
        for x in range(8):
            print("{}: {}".format(x, y_pred[x]))

        # tally the predictions
        value = Counter(y_pred).most_common()
    
        print("{} : {}".format(value[0][0], str(value)))
        sys.stdout.flush()
        
        return value[0][0], value[0][1], self.datapoint_count
    

if __name__ == '__main__':
    # Execute when the module is not initialized from an import statement.
    # this is a test of this class

    # artificial payload for testing only
    payload = "{ \"datapoints\" : [\
    [ 6, 3558025, 38.963810, 1007.948975, 19.903688, 59836568.000000 ],\
    [ 5, 3558165, 39.379066, 1007.898865, 19.253843, 102400000.000000 ],\
    [ 7, 3558167, 38.742668, 1007.842346, 19.633577, 81259768.000000 ],\
    [ 3, 3558443, 39.623634, 1007.606567, 18.959818, 102400000.000000 ],\
    [ 1, 3558862, 38.875629, 1007.776001, 19.469082, 102400000.000000 ],\
    [ 4, 3558865, 39.565823, 1007.952148, 18.836962, 101922240.000000 ],\
    [ 0, 3559281, 38.350483, 1007.676331, 19.998688, 102400000.000000 ],\
    [ 2, 3559703, 39.413464, 1007.699097, 19.325720, 100979968.000000 ]\
    ] }"

    #print(payload)

    model = AirAlcoholCoffeeDetection('enoseSc.pkl', 'enoseModel.pkl')
    smell, sample_correct_Count = model.PredictSmell(payload)
    print("{} : {} out of 8".format(smell, sample_correct_Count))