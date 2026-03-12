import speech_recognition as sr
import json
import os
print("converting")
directory = os.getcwd()
dict = {}
#name of the audio file
file_name = directory + "/miscellaneous_data/audio.wav"

#initiate the recogniser
recogniser = sr.Recognizer()

#open the audio file
with sr.AudioFile(file_name) as audio_file:
    audio_data = recogniser.record(audio_file)
    
    try:
        #use google to convert speech to text
        dict["text"] = recogniser.recognize_google(audio_data)
    #if it cant recognise any text from the audio recorded it will return a question mark
    except:
        dict["text"] = "?"
    
    #write translated text to the json file to be used by c++ files
    with open(directory + "/miscellaneous_data/python_result.json", "w") as file:
        json.dump(dict, file)

print("Done")