#ifndef __INPUTSET__H
#define __INPUTSET__H


namespace inputfiles
{
    class teams;
    class projects;
    class teambacklogs;
    class publicholidays;


    class inputset
    {
        public:
            inputset(projects & p, teams & t, publicholidays & h, teambacklogs & b) : mP(p), mT(t), mH(h), mB(b) {}

        public:
            projects & mP;
            teams & mT;
            publicholidays & mH;
            teambacklogs & mB;
    };

    class constinputset
    {
        public:
            constinputset(const projects & p, const teams & t, const publicholidays & h, const teambacklogs & b) : mP(p), mT(t), mH(h), mB(b) {}
            constinputset(const inputset & other) : mP(other.mP), mT(other.mT), mH(other.mH), mB(other.mB) {}

        public:
            const projects & mP;
            const teams & mT;
            const publicholidays & mH;
            const teambacklogs & mB;
    };
}    



#endif