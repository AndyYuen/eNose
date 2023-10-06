"""
API exposing the eNose odour detection functionality
"""
import argparse
import sys

from flask import Flask, request
from AirAlcoholCoffeeDetection import AirAlcoholCoffeeDetection

app = Flask(__name__)

ENOSE_URL = "/v1/odour-detection"


@app.route(ENOSE_URL, methods=["POST"])
def detect():
    if not (request.method == "POST" and request.is_json):
        return "Must be POST and json", 400

    # debug only
    #print(request.get_json(force=True))


    smell, sample_correct_Count, totoal_count = model.PredictSmell(request.get_json())
    #print("{} : {} out of 8".format(smell, sample_correct_Count))
    sys.stdout.flush()

    result = {}
    result['prediction'] = smell
    result['majorityPredictions'] = sample_correct_Count
    result['totalPredictions'] = totoal_count
    return result


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="Flask api exposing odour detection")
    parser.add_argument("--port", default=7000, type=int, help="port 7000")
    args = parser.parse_args()

    model = AirAlcoholCoffeeDetection('enoseSc.pkl', 'enoseModel.pkl')

    app.run(host="0.0.0.0", port=args.port)  # debug=True causes Restarting with stats
