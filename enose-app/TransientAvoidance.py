class TransientAvoidance:

    # constructor
    def __init__(self,):
        self.odour = "unknown"
        self.threshold = 3
        self.count = 0


    # avoid transient
    def avoidTransient(self, odour):
        if self.odour == "unknown" and self.count < self.threshold:
            self.count += 1
            if self.count == 1:
                return True, "unknown"
            return False, "unknown"
            
        if odour == self.odour:
            self.count += 1
            if self.count == self.threshold:
                return True, odour
            else:
                return False, odour
        else:
            self.odour = odour
            self.count = 0
            return True, "unknown"
        
    # reset object
    def reset(self):
        self.odour = "unknown"
        self.count = 0


if __name__ == '__main__':
    suppress = TransientAvoidance()
    print(suppress.avoidTransient("coffee"))
    print(suppress.avoidTransient("coffee"))
    print(suppress.avoidTransient("coffee"))
    print(suppress.avoidTransient("coffee"))
    print(suppress.avoidTransient("air"))
    print(suppress.avoidTransient("air"))
    print(suppress.avoidTransient("alcohol"))
    print(suppress.avoidTransient("coffee"))
    print(suppress.avoidTransient("coffee"))
    print(suppress.avoidTransient("coffee"))
    print(suppress.avoidTransient("coffee"))
    print(suppress.avoidTransient("alcohol"))
    print(suppress.avoidTransient("alcohol"))
    print(suppress.avoidTransient("alcohol"))
    print(suppress.avoidTransient("alcohol"))