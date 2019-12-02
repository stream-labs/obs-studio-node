const request = require('request');

type TPlatform = 'twitch' | 'youtube' | 'mixer' | 'facebook';

interface ITestUser {
    email: string;
    workerId: string;
    updated: string;
    enabled: boolean;
    type: TPlatform;
    username: string;
    id: string;
    token: string;
    apiToken: string;
    widgetToken: string;
    streamKey: string;
}

export class Services {
    private user: ITestUser;
    private userPoolUrl: string = 'https://slobs-users-pool.herokuapp.com/';
    private clientId = '8bmp6j83z5w4mepq0dn0q1a7g186azi';

    private sleep(ms) {
        return new Promise(resolve => setTimeout(resolve, ms));
    }

    private async requestUser(): Promise<any> {
        return new Promise((resolve, reject) => {
            request({
                url: this.userPoolUrl + 'reserve' + '/twitch',
                headers: {Authorization: `Bearer: ${process.env.SLOBS_TEST_USER_POOL_TOKEN}`}},
                (err: any, res: any, body: any) => {
                    if (err || res.statusCode !== 200) {
                        reject(`Unable to request user ${err || body}`);
                    }

                    if (body == undefined) {
                        reject(`Body is undefined or with wrong format ${body}`);
                    }

                    resolve(JSON.parse(body));
                }
            );
        });
    }

    async getStreamKey(): Promise<string> {
        let attemps: number = 3;

        while(attemps--) {
            try {
                this.user = await this.requestUser();
                break;
            } catch(e) {
                if (attemps) {
                    await this.sleep(20000);
                }
            }
        }

        if (!this.user) {
            throw 'Unable to get user from pool.';
        }

        return this.user.streamKey;
    }

    async releaseUser() {
        return new Promise((resolve, reject) => {
            request({
                url: this.userPoolUrl + `release/${this.user.type}/${this.user.email}`,
                headers: {Authorization: `Bearer: ${process.env.SLOBS_TEST_USER_POOL_TOKEN}`}
            }, (err: any, res: any, body: any) => {
                if (err || res.statusCode !== 200) {
                    reject(`Unable to release user ${err || body}`);
                }

                if (body == undefined) {
                    reject(`Body is undefined or with wrong format ${body}`);
                }

                resolve(JSON.parse(body));
            });
        });
    }
}