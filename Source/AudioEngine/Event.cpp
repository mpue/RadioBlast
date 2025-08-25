//
//  Event.cpp
//  Synthlab - App
//
//  Created by Matthias Pueski on 05.04.18.
//

#include "Event.h"

using juce::String;

static int nextId = 1;
int id = nextId++;

Event::Event() {

}

Event::Event(Event* e){
    this->name = e->getName();
    this->type = e->getType();
    this->value = e->getValue();
    this->note = e->getNote();
    this->number = e->getNumber();
    this->message = e->getMessage();
}

Event::Event(String name, Type type) {
    this->name = name;
    this->type = type;
}

Event::~Event() {
    
}

String Event::getName() {
    return name;
}

void Event::setType(Type type)
{
    this->type = type;
}

Event::Type Event::getType() {
    return type;
}

void Event::setValue(int value) {
    this->value = value;
}

int Event::getValue() {
    return value;
}

void Event::setNote(int note) {
    this->note = note;
}

int Event::getNote(){
    return note;
}

void Event::setNumber(int num) {
    this->number = num;
}

void Event::setMessage(juce::MidiMessage message)
{   
    this->message = message;
}

juce::MidiMessage Event::getMessage()
{
    return message;
}

int Event::getNumber() {
    return number;
}
