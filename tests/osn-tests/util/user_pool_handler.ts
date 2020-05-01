import { sleep } from './general';
import { logInfo, logWarning } from './logger';

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

export class UserPoolHandler {
    private user: ITestUser;
    private userPoolUrl: string = 'https://slobs-users-pool.herokuapp.com/';
    private osnTestName: string;

    constructor(testName: string) {
        this.osnTestName = testName;
    }

    private async requestUser(): Promise<any> {
        return new Promise((resolve, reject) => {
            request({
                url: this.userPoolUrl + 'reserve' + '/twitch',
                headers: {Authorization: `Bearer: ${process.env.SLOBS_TEST_USER_POOL_TOKEN}`}},
                (err: any, res: any, body: any) => {
                    if (err || res.statusCode !== 200) {
                        logWarning(this.osnTestName, 'Request user got status ' + res.statusCode);
                        logWarning(this.osnTestName, 'Error mesage \'' + err + '\'');
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
        let attempt: number = 1;
        let totalAttempts: number = 3;

        while(attempt <= totalAttempts) {
            try {
                logInfo(this.osnTestName, 'Requesting user from pool ('+ attempt + '/' + totalAttempts + ')');
                this.user = await this.requestUser();
                break;
            } catch(e) {
                if (attempt) {
                    await sleep(20000);
                }
            }

            attempt++;
        }

        if (!this.user) {
            throw 'Unable to get user from pool.';
        }

        logInfo(this.osnTestName, 'Got user ' + this.user.email);
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