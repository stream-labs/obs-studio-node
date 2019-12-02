import { sleep } from './general';
import { logInfo, logWarning } from '../util/logger';

const request = require('request');

type TPlatform = 'twitch' | 'youtube' | 'mixer' | 'facebook';

interface ITestUser {
    email: string;
    workerId: string; // null if user is not active right now
    updated: string; // time of the last request for this user
    username: string; // Mixer use username as an id for API requests
    type: TPlatform; // twitch, youtube, etc..
    id: string; // platform userId
    token: string; // platform token
    apiToken: string; // Streamlabs API token
    widgetToken: string; // needs for widgets showing
    channelId?: string; // for the Mixer and Facebook only
}

export class Services {
    private user: ITestUser;
    private userPoolUrl: string = 'https://slobs-users-pool.herokuapp.com/';
    private clientId = '8bmp6j83z5w4mepq0dn0q1a7g186azi';
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

    private async validateToken(token: string) {
        return new Promise((resolve, reject) => {
            request({
                url: 'https://id.twitch.tv/oauth2/validate',
                headers: {'Client-id': this.clientId,
                          Accept: 'application/vnd.twitchtv.v5+json',
                          'Content-Type': 'application/json',
                          Authorization: `OAuth ${token}`}
            }, (err: any, res: any, body: any) => {
                if (err || res.statusCode !== 200) {
                    logWarning(this.osnTestName, 'Validate token got status ' + res.statusCode);
                    logWarning(this.osnTestName, 'Error mesage \'' + err + '\'');
                    reject(`Unable to validate token ${err || body}`);
                }

                if (body == undefined) {
                    reject(`Body is undefined or with wrong format ${body}`);
                }

                resolve(JSON.parse(body));
            });
        });
    }

    private async requestStreamKey(token: string) {
        return new Promise((resolve, reject) => {
            request({
                url: 'https://api.twitch.tv/kraken/channel',
                headers: {'Client-id': this.clientId,
                          Accept: 'application/vnd.twitchtv.v5+json',
                          'Content-Type': 'application/json',
                          Authorization: `OAuth ${token}`}
            }, (err: any, res: any, body: any) => {
                if (err || res.statusCode !== 200) {
                    logWarning(this.osnTestName, 'Request stream key got status ' + res.statusCode);
                    logWarning(this.osnTestName, 'Error mesage \'' + err + '\'');
                    reject(`Unable to get channel info ${err || body}`);
                }

                if (body == undefined) {
                    reject(`Body is undefined or with wrong format ${body}`);
                }

                resolve(JSON.parse(body));
            });
        });
    }

    async getStreamKey(): Promise<string> {
        let attempts: number = 1;
        let totalAttempts: number = 3;
        let channelInfo: any;

        while(attempts <= totalAttempts) {
            try {
                logInfo(this.osnTestName, 'Requesting user from pool ('+ attempts + '/' + totalAttempts + ')');
                this.user = await this.requestUser();
                break;
            } catch(e) {
                if (attempts) {
                    await sleep(20000);
                }
            }

            attempts++;
        }

        if (!this.user) {
            throw 'Unable to get user from pool.';
        }
        
        await this.validateToken(this.user.token);
        channelInfo = await this.requestStreamKey(this.user.token);

        return channelInfo.stream_key;
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